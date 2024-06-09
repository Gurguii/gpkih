#include "actions.hpp"

#include <filesystem>
#include <iostream>
#include <fstream>

using namespace gpkih;

static int __increment_serial(){
  PDEBUG(2,"__increment_serial()");

  std::ifstream rfile(serialPath);
  
  if(!rfile.is_open()){
    PERROR("Couldn't open serial file '{}' to update serial\n", serialPath);
    return GPKIH_FAIL;
  }

  std::string stserial{};
  rfile >> stserial;

  std::string nserial = fmt::format("{:x}",static_cast<decltype(Entity::serial)>(std::stoull(stserial,0,16) + 1));

  PDEBUG(1, "nserial:{}", nserial);

  rfile.close();
  
  std::ofstream wfile(serialPath);
  if(!wfile.is_open()){
    return GPKIH_FAIL;
  }

  if(nserial.size() == 1){
    PDEBUG(1,"adding a zero(0)");
    wfile << "0";
  }

  wfile << nserial;
  
  wfile.close();

  return GPKIH_OK;
}

// Attempts to create the target path (directory or file)
// - if dir != 0 a directory is created and any missing intermediary
// directories will be created too, else a file will be createdPDEBUG
static int __create_outdir(fs::path &path){
  PDEBUG(2,"__create_outdir({})",path.string());

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
  PDEBUG(2, "__server_client_build_commands()");

  Subject &subject = entity.subject;
  // openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc
  
  // if a client key is created and the configuration is changed afterwards,
  // the format taken won't match with the actual format used when the entity
  // was created, leading to errors

  // Set entity certificate request PATH
  std::string tmp = std::move(fmt::format("{}{}-csr.pem",profile::dir_req(profile), subject.cn));
  CALLOCATE(entity.csrPath,reinterpret_cast<size_t*>(&entity.csrPathLen),tmp);

  // Set entity key PATH
  tmp.assign("");
  tmp = std::move(fmt::format("{}{}-key.PEM",profile::dir_key(profile), subject.cn));
  CALLOCATE(entity.keyPath,reinterpret_cast<size_t*>(&entity.keyPathLen),tmp);

  tmp.assign("");

  std::string csr_command = std::move(fmt::format("openssl req -newkey {}:{} -out \"{}\" -keyout \"{}\" -subj {} -noenc",
    keyAlgo,
    keySize,
    entity.csrPath,
    entity.keyPath,
    subject::openssl_oneliner(subject)
  ));
  
  // openssl ca -config {} -in {} -out {} -subj '{}' -extfile {}x509{}{} -notext -days +{}

  // Set entity certificate PATH
  tmp = std::move(fmt::format("{}{}-crt.pem",profile::dir_crt(profile), subject.cn));
  CALLOCATE(entity.crtPath,reinterpret_cast<size_t*>(&entity.crtPathLen),tmp);

  tmp.assign("");
  
  std::string x509_extensions_file_path = CONF_DIRPATH + "x509" + SLASH + to_str(entity.type);
  
  std::string crt_command = std::move(fmt::format("openssl ca -config \"{}\" -in \"{}\" -out \"{}\" -extfile \"{}\" -days {}",
    profile::gopenssl(profile),
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
  PDEBUG(2, "__ca_build_command()");

  std::string command = std::move(fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj {} -set_serial {} -newkey {}:{} -noenc -days {}",
    profile::gopenssl(profile),
    entity.crtPath,
    entity.keyPath,
    subject::openssl_oneliner(entity.subject),
    entity.serial,
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
                                 std::vector<Entity> &entities,bool autoanswer, bool prompt) {
  PDEBUG(2, "__create_inline_config()");
  
  std::string caCertPath = std::move(profile::ca_crt(profile));

  if (!fs::exists(caCertPath)) {
    PERROR("ca cert path '{}' doesn't exist\n", caCertPath);
    return GPKIH_FAIL;
  }

  // Load CA certificate
  size_t caCertSize = fs::file_size(caCertPath);
  //std::string caCertBuffer("\0", caCertSize);
  char *caCertBuffer = ALLOCATE(caCertSize);
  if(caCertBuffer == nullptr){
    PERROR("couldn't load CA certificate '{}'\n", caCertPath);
    return GPKIH_FAIL;  
  }

  std::ifstream(caCertPath).read(caCertBuffer, caCertSize);

  fs::path profile_dir_key = std::move(profile::dir_key_fs(profile));
  fs::path profile_dir_crt = std::move(profile::dir_crt_fs(profile));
  
  for (auto &entity : entities) 
  {
    fs::path entity_directory = profile.source;
    entity_directory /= "packs";
    entity_directory /= entity.subject.cn;

    if (__create_outdir(entity_directory)) {
      return GPKIH_FAIL;
    };

    fs::path entity_config_file = entity_directory / fmt::format("inline_{}.{}",entity.subject.cn,vpnConfigExtension);

    if(config.dump_vpn_conf(entity_config_file, entity.type) == false){
      return GPKIH_FAIL;
    }

    ConfigMap &pkiconf = config.get(CONFIG_PKI);
    
    std::string_view key_extension = pkiconf["key"]["creation_format"];
    std::string_view crt_extension = pkiconf["crt"]["creation_format"];

    if (!fs::exists(entity.keyPath)) {
      PERROR("missing entity '{}' key\n", entity.subject.cn);
      return GPKIH_FAIL;
    }

    if(!fs::exists(entity.crtPath)){
      PERROR("missing entity crt '{}'\n", entity.subject.cn);
      return GPKIH_FAIL;
    }
    
    size_t entity_key_size = fs::file_size(entity.keyPath);
    size_t entity_crt_size = fs::file_size(entity.crtPath);
    
    std::ofstream file(entity_config_file, std::ios::app);
    
    if (!file.is_open()) {
      PERROR("couldn't open entity config file '{}'\n", entity_config_file.string());
      return GPKIH_FAIL;
    }

    std::ifstream ekey(entity.keyPath);
    std::ifstream ecrt(entity.crtPath);

    if (!ekey.is_open()) {
      PERROR("couldn't open entity key file '{}'\n", entity.keyPath);
      return GPKIH_FAIL;
    }

    if (!ecrt.is_open()) {
      PERROR("couldn't open entity certificate file '{}'\n", entity.crtPath);
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

    FREEBLOCK(caCertBuffer);
  }
  return GPKIH_OK;
}

int __create_pfx(Profile &profile, Entity &entity){
  PDEBUG(2, "__create_pfx()");

  fs::path entity_dir = fmt::format("{}{}packs{}{}",profile.source,SLASH,SLASH,entity.subject.cn);

  if(__create_outdir(entity_dir) == GPKIH_FAIL){
    return GPKIH_FAIL;
  };

  std::string command = fmt::format("openssl pkcs12 -export -inkey {} -in {} -out {}.pfx",
    entity.keyPath,entity.crtPath,(entity_dir/entity.subject.cn).string());

  if(system(command.c_str())){
    PERROR("command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

/* BUILD SELF SIGNED CA certificate */
int actions::build_ca(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman,std::string_view days, std::string_view keyAlgo, std::string_view keySize){
  PDEBUG(1,"actions::build_ca()");

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;

  if(autoanswer == false && profile.ca_created){
    auto ans = PROMPT("Profile already has CA and\ncreating a new one will cause newly created certificates not to work with older ones , continue?", "[y/n]", true, LGREEN);
    if(ans != "y" && ans != "yes"){
      return GPKIH_FAIL;
    } 
  }

  if(profile::ca_key(profile, entity) == GPKIH_FAIL || profile::ca_crt(profile, entity) == GPKIH_FAIL){
    return GPKIH_FAIL;
  }; 

  ConfigMap &pkiconf = config.get(CONFIG_PKI);
  std::string command = std::move(__ca_build_command(profile,pkiconf,entity,days,keyAlgo,keySize));
  
  PDEBUG(3, "self-signed CA command - '{}'", command);
  
  if(system(command.c_str())){
    PERROR("command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  __increment_serial();
  
  // Add entity to entity database
  eman.add(entity);

  // Update profile modification date + ca_created status
  profile.last_modification = std::chrono::system_clock::now();
  profile.ca_created = 1;

  // Synchronize so that changes take effect
  db::profiles::sync();
  
  eman.sync();

  //PSUCCESS("Certificate authority '{}' created\n", entity.subject.cn);
  //ADD_LOG(L_INFO, "profile:{} action:build type:{}", profile.name, to_str(entity.type));

  return GPKIH_OK;
} // actions::build_ca()

/* BUILD SERVER-CLIENT CERTIFICATES */
int actions::build(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman, std::string_view days, std::string_view keyAlgo, std::string_view keySize){
  PDEBUG(1,"actions::build()");


  ConfigMap &pkiconf = config.get(CONFIG_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;
  
  const auto [req_command, crt_command] = __server_client_build_commands(profile, pkiconf, entity, days, keyAlgo, keySize);

  // Create key + csr
  PDEBUG(2,"executing key + certificate request command - {}",req_command);
  if(system(req_command.c_str())){
    // fail
    PERROR("[ REQUEST ] command '{}' failed\n",req_command);
    return GPKIH_FAIL;
  }

  // Create crt
  PDEBUG(2,"executing certificate command - {}",crt_command);
  if(system(crt_command.c_str())){
    // fail
    PERROR("[ CERTIFICATE SIGNING ] command '{}' failed\n",crt_command);
    return GPKIH_FAIL;
  }

  // Add entity to database
  eman.add(entity);
  
  // Update modification time + client/server count
  profile.last_modification = std::chrono::system_clock::now();
  switch(entity.type){
    case ET_SV:
      ++profile.sv_count;
      break;
    case ET_CL:
      ++profile.cl_count;
      break;
    default:
      break;
  }

  // Synchronize so that changes take effect
  db::profiles::sync();
  eman.sync();
  
  // Create inline config file
  std::vector<Entity> entityList{entity};

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