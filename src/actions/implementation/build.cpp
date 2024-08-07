#include "../actions.hpp"
#include "../../entities/conv.hpp"
#include "../../entities/subj_utils.hpp"
#include "../../config/Config.hpp"

using namespace gpkih;

#ifdef _WIN32
constexpr const char *vpnConfigExtension = "ovpn";
#else
constexpr const char *vpnConfigExtension = "conf";
#endif
// Attempts to create the target path (directory or file)
// - if dir != 0 a directory is created and any missing intermediary
// directories will be created too, else a file will be createdDEBUGF
static int __create_outdir(fs::path &path){
  DEBUGF(2,"__create_outdir({})",path.string());

  if(path.is_relative()){
    PERROR("path to __create_outdir() must be absolute, given path: {}", path.string());
    return GPKIH_FAIL;
  }

  if(fs::exists(path) == false){
    return fs::create_directory(path) ? GPKIH_OK : GPKIH_FAIL;
  }else if(fs::is_directory(path) == false){
    fs::remove_all(path);
    return fs::create_directory(path) ? GPKIH_OK : GPKIH_FAIL;  
  }

  return GPKIH_OK;
};

static std::pair<std::string,std::string> __server_client_build_commands(Profile &profile, ConfigMap &pkiconf,
                                             Entity &entity, std::string_view days, std::string_view keyAlgo, std::string_view keySize) {
  DEBUG(2, "__server_client_build_commands()");

  Subject &subject = entity.subject;
  // openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc
  
  // STICK TO PEM HIJO DE PUTA

  // Set entity certificate request PATH
  std::string tmp = std::move(fmt::format("{}{}-csr.pem",profile::reqDir(profile), subject.cn));

  // Set entity key PATH
  tmp.assign("");
  tmp = std::move(fmt::format("{}{}-key.pem",profile::keyDir(profile), subject.cn));

  std::string csr_command = std::move(fmt::format("openssl req -newkey {}:{} -out \"{}\" -keyout \"{}\" -subj {} -noenc",
    keyAlgo,
    keySize,
    entity.csrPath,
    entity.keyPath,
    subject::opensslOneliner(subject)
  ));
  
  // Set entity certificate PATH
  tmp = std::move(fmt::format("{}{}-crt.pem",profile::crtDir(profile), subject.cn));

  tmp.assign("");
  
  std::string x509_extensions_file_path = CONF_DIRPATH + "x509" + SLASH + entity::conversion::toString(entity.meta.type);
  
  std::string crt_command = std::move(fmt::format("openssl ca -config \"{}\" -in \"{}\" -out \"{}\" -extfile \"{}\" -days {}",
    profile::gopensslPath(profile),
    entity.csrPath,
    entity.crtPath,
    x509_extensions_file_path,
    days
  ));

  // Check additional opts based on config
  if(Config::get("behaviour","autoanswer") == "yes"){
    crt_command += " -batch";
  }
    
  if(Config::get("behaviour","print_generated_certificate") == "no"){
    crt_command += " -notext";
  }

  return {std::move(csr_command),std::move(crt_command)};
}

static std::string __ca_build_command(Profile &profile, ConfigMap &pkiconf, Entity &entity, std::string_view days, std::string_view keyAlgo, std::string_view keySize){
  DEBUG(2, "__ca_build_command()");

  std::string command = std::move(fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj {} -set_serial {} -newkey {}:{} -noenc -days {}",
    profile::gopensslPath(profile),
    entity.crtPath,
    entity.keyPath,
    subject::opensslOneliner(entity.subject),
    entity.meta.serial,
    keyAlgo,
    keySize,
    days
  ));
  
  // Check for optional arguments based on configuration
  if(Config::get("behaviour","autoanswer") == "yes"){
    command += " -batch"; // doesn't ask before signing certificate or updating database
  }

  return std::move(command);
};

static int __create_inline_config(Profile &profile,ProfileConfig &config,
                                 std::vector<const Entity*> &entities,bool autoanswer, bool prompt) {
  DEBUG(2, "__create_inline_config()");
  std::string caCertPath = std::move(profile::caCertificatePath(profile));

  if (!fs::exists(caCertPath)) {
    PERROR("ca cert path '{}' doesn't exist\n", caCertPath);
    return GPKIH_FAIL;
  }

  // Load CA certificate
  size_t caCertSize = fs::file_size(caCertPath);
  char *caCertBuffer = (char*)ALLOCATE(caCertSize);

  if(caCertBuffer == nullptr){
    PERROR("couldn't load CA certificate '{}'\n", caCertPath);
    return GPKIH_FAIL;  
  }

  std::ifstream(caCertPath).read(caCertBuffer, caCertSize);

  for (auto &entity : entities) 
  {
    auto entityDir = fs::path{profile.source} / "packs" / entity->subject.cn;
    if (__create_outdir(entityDir) == GPKIH_FAIL) {
      return GPKIH_FAIL;
    };

    auto entityInlinePath = entityDir / fmt::format("inline_{}.{}",entity->subject.cn,vpnConfigExtension);

    if(config.dump_vpn_conf(entityInlinePath, entity->meta.type) == false){
      return GPKIH_FAIL;
    }

    if (!fs::exists(entity->keyPath)) {
      PERROR("Missing entity key '{}'\n", entity->subject.cn);
      return GPKIH_FAIL;
    }

    if(!fs::exists(entity->crtPath)){
      PERROR("Missing entity crt '{}'\n", entity->subject.cn);
      return GPKIH_FAIL;
    }
    
    std::ofstream file(entityInlinePath, std::ios::app);
    
    if (!file.is_open()) {
      PERROR("Couldn't open entity config file '{}'\n", entityInlinePath.string());
      return GPKIH_FAIL;
    }

    std::ifstream ekey(entity->keyPath);
    std::ifstream ecrt(entity->crtPath);

    if (!ekey.is_open()) {
      PERROR("Couldn't open entity key file '{}'\n", entity->keyPath);
      return GPKIH_FAIL;
    }

    if (!ecrt.is_open()) {
      PERROR("Couldn't open entity certificate file '{}'\n", entity->crtPath);
      return GPKIH_FAIL;
    }

    // add inline entity key
    file << "<key>" << EOL << ekey.rdbuf() << "</key>" << EOL;
    // add inline entity certificate
    file << "<crt>" << EOL << ecrt.rdbuf() << "</crt>" << EOL;
    // add inline ca certificate
    file << "<ca>" << EOL << caCertBuffer << "</ca>" << EOL;

    ecrt.close();
    ekey.close();

    FREE_MEMORY_BLOCK((std::byte*)caCertBuffer, &caCertSize);
  }
  return GPKIH_OK;
}

int __create_pfx(Profile &profile, Entity &entity){
  DEBUG(2, "__create_pfx()");

  auto entityDir = fs::path{profile.source}/"packs"/entity.subject.cn;
  auto outpathPfx = entityDir / entity.subject.cn;

  if(__create_outdir(entityDir) == GPKIH_FAIL){
    return GPKIH_FAIL;
  };

  std::string command = fmt::format("openssl pkcs12 -export -inkey {} -in {} -out {}.pfx",
    entity.keyPath,entity.crtPath,outpathPfx.string());

  if(system(command.c_str())){
    PERROR("Command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

/* BUILD SELF SIGNED CA certificate */
int actions::build_ca(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman,std::string_view days, std::string_view keyAlgo, std::string_view keySize){
  DEBUG(1,"actions::build_ca()");

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;

  if(autoanswer == false && profile.meta.caCreated){
    auto ans = PROMPT("Profile already has CA and\ncreating a new one will cause newly created certificates not to work with older ones , continue?", "[y/n]", true, LGREEN);
    if(ans != "y" && ans != "yes"){
      return GPKIH_FAIL;
    } 
  }

  ConfigMap &pkiconf = config.get(CFILE_PKI);
  std::string command = std::move(__ca_build_command(profile,pkiconf,entity,days,keyAlgo,keySize));
  
  DEBUGF(3, "self-signed CA command - '{}'", command);
  
  if(system(command.c_str())){
    PERROR("command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  entity::incrementSerial(profile, entity);
  
  // Update profile modification date + caCreated status
  profile.meta.lastModification = std::chrono::system_clock::now();
  profile.meta.caCreated = 1;

  return GPKIH_OK;
} // actions::build_ca()

/* BUILD SERVER-CLIENT CERTIFICATES */
int actions::build(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman, std::string_view days, std::string_view keyAlgo, std::string_view keySize){
  DEBUG(1,"actions::build()");

  ConfigMap &pkiconf = config.get(CFILE_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  
  const auto [req_command, crt_command] = __server_client_build_commands(profile, pkiconf, entity, days, keyAlgo, keySize);

  // Create key + csr
  DEBUGF(2,"executing key + certificate request command - {}",req_command);
  if(system(req_command.c_str())){
    // fail
    PERROR("[ REQUEST ] command '{}' failed\n",req_command);
    return GPKIH_FAIL;
  }

  // Create crt
  DEBUGF(2,"executing certificate command - {}",crt_command);
  if(system(crt_command.c_str())){
    // fail
    PERROR("[ CERTIFICATE SIGNING ] command '{}' failed\n",crt_command);
    return GPKIH_FAIL;
  }

  // Update modification time + client/server count
  profile.meta.lastModification = std::chrono::system_clock::now();

  switch(entity.meta.type){
    case ET_SV:
      ++profile.meta.svCount;
      break;
    case ET_CL:
      ++profile.meta.clCount;
      break;
    default:
      break;
  }

  // Create inline config file
  std::vector<const Entity*> entityList{&entity};

  bool create_inline = pkiconf["output"]["create_inline"] == "yes" ? true : false;
  bool create_pfx = pkiconf["output"]["create_pfx"] == "yes" ? true : false;

  if(create_inline && __create_inline_config(profile, config, entityList, autoanswer, prompt) == GPKIH_OK){
    PSUCCESS("inline config created\n");
  }

  if(create_pfx &&  __create_pfx(profile,entity) == GPKIH_OK){
    PSUCCESS("pfx file created\n");
  }
  
  return GPKIH_OK;
}