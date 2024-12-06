#include "add.hpp"

#include "../../config/Config.hpp"

#include "../../gpkih.hpp"

#include "../../profiles/profiles.hpp"

#include "../../entities/subj_utils.hpp"
#include "../../entities/conv.hpp"

#include "../../libs/printing/printing.hpp"

using namespace gpkih;

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

  std::string csr_command = std::move(fmt::format("openssl req -newkey {}:{} -out \"{}\" -keyout \"{}\" -subj {} -noenc",
    keyAlgo,
    keySize,
    entity.csrPath,
    entity.keyPath,
    subject::opensslOneliner(subject)
  ));
  
  std::string x509_extensions_file_path = GPKIH_DIR_CONFIG + "x509" + SLASH + entity::conversion::toString(entity.meta.type);
  
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
    file << "<cert>" << EOL << ecrt.rdbuf() << "</cert>" << EOL;
    // add inline ca certificate
    file << "<ca>" << EOL << caCertBuffer << "</ca>" << EOL;

    auto dhPath = fs::path(profile.source) / "pki/tls/dhparam";

    if(entity->meta.type == ET_SV && fs::exists(dhPath)){
      size_t dhSize = fs::file_size(dhPath);
      auto dh = (char*)ALLOCATE(dhSize);
      
      if(dh == nullptr){
        PERROR("Couldn't allocate data - {}\n", gpkihBuffer->getLastError());
        return GPKIH_FATAL;
      }

      std::ifstream(dhPath).read(dh, dhSize);

      file << "<dh>" << EOL << dh << "</dh>" << EOL;
      FREE_MEMORY_BLOCK(dh, dhSize);
    }

    ecrt.close();
    ekey.close();

    FREE_MEMORY_BLOCK(caCertBuffer, caCertSize);
  }
  return GPKIH_OK;
}

static int __create_pfx(Profile &profile, Entity &entity){
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

int actions::add::add(Profile &profile, 
	ProfileConfig &config, 
	Entity &entity, EntityManager &eman, 
	std::string_view days, 
	std::string_view keyAlgo, 
	std::string_view keySize, 
	bool autoanswer, 
	bool prompt
){
	DEBUG(1,"actions::build()");

  ConfigMap &pkiconf = config.get(CFILE_PKI);

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