#include "actions.hpp"

#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace gpkih;


static int __increment_serial(){
  PDEBUG(1,"__increment_serial()");

  std::fstream file(serial_path, std::ios::in | std::ios::out);
  
  if(!file.is_open()){
    seterror("couldn't open serial file '{}' to update serial\n", serial_path);
    return F_NOCREATE;
  }

  std::string stserial{};
  file >> stserial;
  std::string nserial = fmt::format("{:x}",static_cast<decltype(Entity::serial)>(std::stoi(stserial) + 1));
  PDEBUG(1, "NEW SERIAL: {}\n", nserial);
  file.seekp(SEEK_SET);
  
  if(nserial.size() == 1){
    file << "0";
  }

  file << nserial;
  return GPKIH_OK;
}

// Attempts to create the target path (directory or file)
// - if dir != 0 a directory is created and any missing intermediary
// directories will be created too, else a file will be created
static int __create_outdir(fs::path &path){
  PDEBUG(1,"__create_outdir({})",path.string());

  if(path.is_relative()){
    seterror("path to __create_outdir() must be absolute, given path: {}", path.string());
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
                                             Entity &entity) {
  PDEBUG(1, "__server_client_build_commands()");

  Subject &subject = entity.subject;
  // openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc
  std::string_view key_algo = pkiconf["key"]["algorithm"];
  std::string_view key_size = pkiconf["key"]["size"];
  
  // TODO - add entity paths to the csv when created
  // if a client key is created and the configuration is changed afterwards,
  // the format taken won't match with the actual format used when the entity
  // was created, leading to errors

  std::string_view csr_creation_format = pkiconf["csr"]["creation_format"];
  std::string_view key_creation_format = pkiconf["key"]["creation_format"];

  // Set entity certificate request PATH
  std::string tmp = std::move(fmt::format("{}{}-csr.{}",profile::dir_req(profile), subject.cn, csr_creation_format.data()));
  CALLOCATE(entity.csr_path,reinterpret_cast<size_t*>(&entity.csr_path_len),tmp);

  // Set entity key PATH
  tmp.assign("");
  tmp = std::move(fmt::format("{}{}-key.{}",profile::dir_key(profile), subject.cn, key_creation_format.data()));
  CALLOCATE(entity.key_path,reinterpret_cast<size_t*>(&entity.key_path_len),tmp);

  tmp.assign("");

  std::string csr_command = std::move(fmt::format("openssl req -newkey {}:{} -out \"{}\" -keyout \"{}\" -subj {} -outform {} -keyform {} -noenc",
    key_algo,
    key_size,
    entity.csr_path,
    entity.key_path,
    subject::openssl_oneliner(subject),
    csr_creation_format,
    key_creation_format
  ));
  
  // openssl ca -config {} -in {} -out {} -subj '{}' -extfile {}x509{}{} -notext -days +{}
  std::string_view crt_creation_format = pkiconf["crt"]["creation_format"];

  // Set entity certificate PATH
  tmp = std::move(fmt::format("{}{}-crt.{}",profile::dir_crt(profile), subject.cn, crt_creation_format.data()));
  CALLOCATE(entity.crt_path,reinterpret_cast<size_t*>(&entity.crt_path_len),tmp);

  tmp.assign("");
  
  std::string x509_extensions_file_path = CONF_DIRPATH + "x509" + SLASH + to_str(entity.type);
  std::string_view days = pkiconf["crt"]["days"];
  
  std::string crt_command = std::move(fmt::format("openssl ca -config \"{}\" -in \"{}\" -out \"{}\" -extfile \"{}\" -days +{}",
    profile::gopenssl(profile),
    entity.csr_path,
    entity.crt_path,
    x509_extensions_file_path,
    days
  ));

  // Check additional opts based on config
  if(Config::get("behaviour","autoanswer") == "yes"){
    crt_command += " -batch";
  }
  
  return {std::move(csr_command),std::move(crt_command)};
}

static std::string __ca_build_command(Profile &profile, ConfigMap &pkiconf, Entity &entity){
  PDEBUG(1, "__ca_build_command()");

  std::string ca_crt_path = profile::ca_crt(profile);
  std::string ca_key_path = profile::ca_key(profile);

  std::string command = std::move(fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj {} -set_serial {} -newkey {}:{} -noenc",
    profile::gopenssl(profile),
    ca_crt_path,
    ca_key_path,
    subject::openssl_oneliner(entity.subject),
    entity.serial,
    pkiconf["key"]["algorithm"],
    pkiconf["key"]["size"]
  ));
  
  // Check for optional arguments based on configuration
  if(Config::get("behaviour","autoanswer") == "yes"){
    command += " -batch"; // doesn't ask before signing certificate or updating database
  }
  
  return std::move(command);
};

static int __create_inline_config(Profile &profile,ProfileConfig &config,
                                 std::vector<Entity> &entities,bool autoanswer, bool prompt) {
  PDEBUG(1, "__create_inline_config()");
  
  std::string ca_crt_path = std::move(profile::ca_crt(profile));

  if (!fs::exists(ca_crt_path)) {
    seterror("ca cert path '{}' doesn't exist\n", ca_crt_path);
    return GPKIH_FAIL;
  }

  // Load CA certificate
  size_t _ca_filesize = fs::file_size(ca_crt_path);
  //std::string ca_crt_str("\0", _ca_filesize);
  char *ca_crt_str = ALLOCATE(_ca_filesize);
  if(ca_crt_str == nullptr){
    seterror("couldn't load CA certificate '{}'\n", ca_crt_path);
    return GPKIH_FAIL;  
  }

  std::ifstream(ca_crt_path).read(ca_crt_str, _ca_filesize);

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

    fs::path entity_config_file = entity_directory / fmt::format("inline_{}.{}",entity.subject.cn,VPN_CONFIG_EXTENSION);

    if(config.dump_vpn_conf(entity_config_file, entity.type) == false){
      return GPKIH_FAIL;
    }

    ConfigMap &pkiconf = config._get(CONFIG_PKI);
    
    std::string_view key_extension = pkiconf["key"]["creation_format"];
    std::string_view crt_extension = pkiconf["crt"]["creation_format"];

    if (!fs::exists(entity.key_path)) {
      seterror("missing entity '{}' key\n", entity.subject.cn);
      return GPKIH_FAIL;
    }

    if(!fs::exists(entity.crt_path)){
      seterror("missing entity crt '{}'\n", entity.subject.cn);
      return GPKIH_FAIL;
    }
    
    size_t entity_key_size = fs::file_size(entity.key_path);
    size_t entity_crt_size = fs::file_size(entity.crt_path);
    
    std::ofstream file(entity_config_file, std::ios::app);
    
    if (!file.is_open()) {
      seterror("couldn't open entity config file '{}'\n", entity_config_file.string());
      return GPKIH_FAIL;
    }

    std::ifstream ekey(entity.key_path);
    std::ifstream ecrt(entity.crt_path);

    if (!ekey.is_open()) {
      seterror("couldn't open entity key file '{}'\n", entity.key_path);
      return GPKIH_FAIL;
    }

    if (!ecrt.is_open()) {
      seterror("couldn't open entity certificate file '{}'\n", entity.crt_path);
      return GPKIH_FAIL;
    }

    // add inline entity key
    file << "<key>" << EOL << ekey.rdbuf() << "</key>" << EOL;
    // add inline entity certificate
    file << "<crt>" << EOL << ecrt.rdbuf() << "</crt>" << EOL;
    // add inline ca certificate
    file << "<ca>" << EOL << ca_crt_str << "</ca>" << EOL;

    ecrt.close();
    ekey.close();
  }
  return GPKIH_OK;
}

int __create_pfx(Profile &profile, Entity &entity){
  PDEBUG(1, "__create_pfx()");

  fs::path entity_dir = fmt::format("{}{}packs{}{}",profile.source,SLASH,SLASH,entity.subject.cn);

  if(__create_outdir(entity_dir) == GPKIH_FAIL){
    return GPKIH_FAIL;
  };

  std::string command = fmt::format("openssl pkcs12 -export -inkey {} -in {} -out {}.pfx",
    entity.key_path,entity.crt_path,(entity_dir/entity.subject.cn).string());

  if(system(command.c_str())){
    seterror("command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

/* BUILD SELF SIGNED CA certificate */
int actions::build_ca(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman){
  PDEBUG(1,"actions::build_ca()");

  if(profile.ca_created){
    auto ans = PROMPT("profile already has CA and\ncreating a new one will cause newly created certificates not to work with older ones , continue?", "[y/n]", true, LGREEN);
    if(ans != "y" && ans != "yes"){
      return GPKIH_FAIL;
    } 
  }

  ConfigMap &pkiconf = config._get(CONFIG_PKI);
  std::string command = std::move(__ca_build_command(profile,pkiconf,entity));

  PDEBUG(2, "running CA key + self-signed certificate command '{}'",command);

  if(system(command.c_str())){
    seterror("command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  __increment_serial();
  
  // Add entity to database
  eman.add(entity);

  // Update profile modification date + ca_created status
  // TODO - handle cases where the CA has already been created, notice the user cause 
  // further created certificates will have a different CA, therefore client certificates created
  // with a different ca won't work with previous CA server certificates (e.g when authenticating to an OVPN sv)
  profile.last_modification = std::chrono::system_clock::now();
  profile.ca_created = 1;

  // Synchronize so that changes take effect
  db::profiles::sync();
  
  eman.sync();

  PSUCCESS("entity '{}' created\n", entity.subject.cn);
  return GPKIH_OK;
} // actions::build_ca()

/* BUILD SERVER-CLIENT CERTIFICATES */
int actions::build(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman){
  PDEBUG(1,"actions::build()");

  if(profile.ca_created == false){
    auto ans = PROMPT("Certificate authority (CA) hasn't been created yet, create?","[y/n]");
    if(ans == "y" || ans == "yes"){
      return build_ca(profile, config, entity, eman);
    }else{
      seterror("can't create server/client certificates without CA");
      return GPKIH_FAIL;
    }
  }

  ConfigMap &pkiconf = config._get(CONFIG_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;

  const auto [req_command, crt_command] = __server_client_build_commands(profile, pkiconf, entity);

  // Create key + csr
  PDEBUG(2,"executing key + certificate request command - {}",req_command);
  if(system(req_command.c_str())){
    // fail
    seterror("[ REQUEST ] command '{}' failed\n",req_command);
    return GPKIH_FAIL;
  }

  // Create crt
  PDEBUG(2,"executing certificate command - {}",crt_command);
  if(system(crt_command.c_str())){
    // fail
    seterror("[ CERTIFICATE SIGNING ] command '{}' failed\n",crt_command);
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

  PSUCCESS("{} entity '{}' created\n", str_conversion(entity.type),entity.subject.cn);
  ADD_LOG(L_INFO, "added entity [profile:{},serial:{},cn:{},type:ca]",profile.name, entity.serial, entity.subject.cn);
  
  // Create inline config file
  std::vector<Entity> hahahah{entity};

  bool create_inline = pkiconf["output"]["create_inline"] == "yes" ? true : false;
  bool create_pfx = pkiconf["output"]["create_pfx"] == "yes" ? true : false;

  if(create_inline && __create_inline_config(profile, config, hahahah, autoanswer, prompt) == GPKIH_OK){
    PSUCCESS("inline config created\n");
  }

  if(create_pfx &&  __create_pfx(profile,entity) == GPKIH_OK){
    PSUCCESS("pfx file created\n");
  }
  
  return GPKIH_OK;
}