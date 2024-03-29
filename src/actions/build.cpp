#include "actions.hpp"
#include <filesystem>
#include <algorithm>
#include <iostream>
#include <fstream>

using namespace gpkih;

// Attempts to create the target path (directory or file)
// - if dir != 0 a directory is created and any missing intermediary
// directories will be created too, else a file will be created
static int create_output_path(fs::path &path, int dir){
  if(path.is_relative()){
    seterror("path to create_output_path() must be absolute, given path: {}", path.string());
    return GPKIH_FAIL;
  }

  if(fs::exists(path)){
    str ans;
    PROMPT("path '" + path.string() + "'exists, remove?","[y/n]");
    getline(std::cin,ans);
    for(char &c : ans){
      c = std::tolower(c);
    }
    if(ans == "n" || ans == "no"){
      return -1;
    }
    try{
      fs::remove_all(path);
    }catch(fs::filesystem_error err){
      PERROR(err.what());
      return -1;
    }
  }else{
    if(dir){
      if(!fs::create_directories(path)){
        PERROR("couldn't create directory '{}'\n",path.string());
        return GPKIH_FAIL;
      }
    }else{
      if(std::ofstream(path).is_open()){
        return GPKIH_OK;
      }      
      return GPKIH_OK;
    }
  }
  // Directory succesfully created
  return GPKIH_OK;
};

// 
static std::pair<str,str> load_entity_files(Entity entity){
  strview entity_crt_path = entity.crt_path;
  strview entity_key_path = entity.key_path;
  // Check file existance
  if(!fs::exists(entity_crt_path)){
    seterror("entity cert file doesn't exist - '{}'", entity_crt_path);
    return {};
  }
  if(!fs::exists(entity_key_path)){
    seterror("entity key file doesn't exist - '{}'", entity_key_path);
    return {};
  }
  // Create fixed size buffers
  std::string crt_buff("\0",fs::file_size(entity_crt_path));
  std::string key_buff("\0",fs::file_size(entity_key_path));
  // Load file contents on buffer
  std::ifstream(entity_crt_path.data()).read(&crt_buff[0], crt_buff.size());
  std::ifstream(entity_key_path.data()).read(&key_buff[0], key_buff.size());
  // Return buffers
  return {std::move(crt_buff),std::move(key_buff)};
}

static std::pair<str,str> _server_client_build_commands(Profile &profile, ConfigMap &pkiconf,
                                             Entity &entity) {
  Subject &subject = entity.subject;
  // openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc
  strview key_algo = pkiconf["key"]["algorithm"];
  strview key_size = pkiconf["key"]["size"];
  
  // TODO - add entity paths to the csv when created
  // if a client key is created and the configuration is changed afterwards,
  // the format taken won't match with the actual format used when the entity
  // was created, leading to errors

  strview csr_creation_format = pkiconf["csr"]["creation_format"];
  strview key_creation_format = pkiconf["key"]["creation_format"];

  entity.csr_path = std::move(fmt::format("{}{}-csr.{}",profile.dir_req(), subject.cn, csr_creation_format.data()));
  entity.key_path = std::move(fmt::format("{}{}-key.{}",profile.dir_key(), subject.cn, key_creation_format.data()));

  str csr_command = std::move(fmt::format("openssl req -newkey {}:{} -out \"{}\" -keyout \"{}\" -subj {} -outform {} -keyform {} -noenc",
    key_algo,
    key_size,
    entity.csr_path,
    entity.key_path,
    subject.oneliner(),
    csr_creation_format,
    key_creation_format
  ));
  
  // openssl ca -config {} -in {} -out {} -subj '{}' -extfile {}x509{}{} -notext -days +{}
  strview crt_creation_format = pkiconf["crt"]["creation_format"];
  entity.crt_path = std::move(fmt::format("{}{}-crt.{}",profile.dir_crt(), subject.cn, crt_creation_format.data()));
  str x509_extensions_file_path = CONF_DIRPATH + "x509" + SLASH + to_str(entity.type);
  strview days = pkiconf["crt"]["days"];
  
  str crt_command = std::move(fmt::format("openssl ca -config \"{}\" -in \"{}\" -out \"{}\" -extfile \"{}\" -days +{}",
    profile.gopenssl(),
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

static str _ca_build_command(Profile &profile, ConfigMap &pkiconf, Entity &entity){
  // not implemented since ca_created
  // its not getting updated in the csv when a new ca is added
  // same goes for sv_count and cl_count
  if(profile.ca_created){
    // todo - add proper handle for this case
    return "";
  }

  str ca_crt_path = fmt::format("{}{}pki{}ca{}crt",profile.source,SLASH,SLASH,SLASH);
  str ca_key_path = fmt::format("{}{}pki{}ca{}key",profile.source,SLASH,SLASH,SLASH);

  str command = std::move(fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj {} -set_serial {} -newkey {}:{} -noenc",
    profile.gopenssl(),
    ca_crt_path,
    ca_key_path,
    entity.subject.oneliner(),
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
  str ca_crt_path = std::move(profile.ca_crt());

  if (!fs::exists(ca_crt_path)) {
    seterror("ca cert path '{}' doesn't exist\n", ca_crt_path);
    return GPKIH_FAIL;
  }

  // Load CA certificate
  // TODO - do std::stringstream and load contents with rdbuf()
  size_t _ca_filesize = fs::file_size(ca_crt_path);
  str ca_crt_str("\0", _ca_filesize);
  std::ifstream(ca_crt_path).read(&ca_crt_str[0], ca_crt_str.size());

  if (ca_crt_str.empty()) {
    seterror("couldn't load CA certificate '{}'\n", ca_crt_path);
    return GPKIH_FAIL;
  }

  fs::path profile_dir_key = std::move(profile.dir_key());
  fs::path profile_dir_crt = std::move(profile.dir_crt());
  
  for (auto &entity : entities) 
  {

    fs::path entity_directory = profile.source;
    entity_directory /= "packs";
    entity_directory /= entity.subject.cn;

    //PINFO("entity directory: {}", entity_directory.string());
    if (create_output_path(entity_directory, 1)) {
      return GPKIH_FAIL;
    };

    fs::path entity_config_file = entity_directory /= fmt::format("inline_{}.{}",entity.subject.cn,VPN_CONFIG_EXTENSION);

    if(config.dump_vpn_conf(entity_config_file, entity.type) == false){
      return GPKIH_FAIL;
    }

    ConfigMap &pkiconf = config._get(CONFIG_PKI);
    
    strview key_extension = pkiconf["key"]["creation_format"];
    strview crt_extension = pkiconf["crt"]["creation_format"];

    // Add entity certificate + key + CA certificate to file
    fs::path entity_key_path = profile_dir_key /= fmt::format("{}-key.{}", entity.subject.cn, key_extension);
    fs::path entity_crt_path = profile_dir_crt /= fmt::format("{}-crt.{}", entity.subject.cn, crt_extension);

    if (!fs::exists(entity_key_path)) {
        seterror("missing entity key '{}'\n", entity.subject.cn);
        return GPKIH_FAIL;
    }

    if(!fs::exists(entity_crt_path)){
        seterror("missing entity crt '{}'\n", entity.subject.cn);
        return GPKIH_FAIL;
    }
    
    size_t entity_key_size = fs::file_size(entity_key_path);
    size_t entity_crt_size = fs::file_size(entity_crt_path);
    
    std::ofstream file(entity_config_file, std::ios::app);
    
    if (!file.is_open()) {
        seterror("couldn't open entity config file '{}'\n", entity_config_file.string());
        return GPKIH_FAIL;
    }

    std::ifstream ekey(entity_key_path);
    std::ifstream ecrt(entity_crt_path);

    if (!ekey.is_open()) {
        seterror("couldn't open entity key file '{}'\n", entity_key_path.string());
        return GPKIH_FAIL;
    }

    if (!ecrt.is_open()) {
        seterror("couldn't open entity certificate file '{}'\n", entity_crt_path.string());
        return GPKIH_FAIL;
    }

    // add entity key
    file << "<key>" << EOL << ekey.rdbuf() << "</key>" << EOL;
    // add entity certificate
    file << "<crt>" << EOL << ecrt.rdbuf() << "</crt>" << EOL;
    // add ca certificate
    file << "<ca>" << EOL << ca_crt_str.c_str() << "</ca>" << EOL;

    ecrt.close();
    ekey.close();
  }
  return GPKIH_OK;
}

static inline int __create_inline_config(str &profile_name,ProfileConfig &config,
                                 std::vector<str> &common_names, bool autoanswer, bool prompt) {
  Profile profile;
  if (db::profiles::load(profile_name, profile)) {
    seterror("couldn't load profile '{}'\n", profile_name);
    return GPKIH_FAIL;
  }
  std::vector<Entity> entities;
  for (const auto &cn : common_names) {
    Entity e;
    if (db::entities::load(profile_name, cn, e)) {
      seterror("couldn't load entity with cn '{}' from profile '{}'\n", cn,
               profile_name);
      return GPKIH_FAIL;
    };
    entities.emplace_back(e);
  }
  return __create_inline_config(profile, config, entities, autoanswer, prompt);
}

/* BUILD SELF SIGNED CA certificate */
int actions::build_ca(Profile &profile, ProfileConfig &config, Entity &entity){
  ConfigMap &pkiconf = config._get(CONFIG_PKI);
  str command = std::move(_ca_build_command(profile,pkiconf,entity));

  if(system(command.c_str())){
    seterror("command '{}' failed\n", command);
    return GPKIH_FAIL;
  }

  // increment serial and write it back to the serial file
  str nserial = fmt::format("{:x}",std::stoi(entity.serial)+1);
  str filepath = profile.source + SLASH + "pki" + SLASH + "serial" + SLASH + "serial";
  std::fstream serial_file(filepath, std::ios::in | std::ios::out);
  if(!serial_file.is_open()){
    seterror("couldn't open serial file '{}' to update serial\n", filepath);
    return F_NOCREATE;
  }
  // Set file pointer to the beginning
  serial_file.seekg(SEEK_SET);

  // If the serial number is <= 15 (0-f) the resulting
  // formatted hex will be 1 char and will cause openssl
  // to fail, so just add the 0 ourselves (tried fmt :x fixing, but 0x01 syntax doesn't work for openssl either)
  if(nserial.size() == 1){
    serial_file << "0";
  }
  serial_file << nserial;
  
  serial_file.close();
  
  // Add to database
  if(db::entities::add(profile.name,entity)){
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

/* BUILD SERVER-CLIENT CERTIFICATES */
int actions::build(Profile &profile, ProfileConfig &config, Entity &entity){
  
  ConfigMap &pkiconf = config._get(CONFIG_PKI);

  bool autoanswer = Config::get("behaviour","autoanswer") == "yes" ? true : false;
  bool prompt = Config::get("behaviour","prompt") == "yes" ? true : false;

  const auto [req_command, crt_command] = _server_client_build_commands(profile, pkiconf, entity);

  // create key + csr
  //fmt::print("{}\n{}\n", req_command, crt_command);

  if(system(req_command.c_str())){
    // fail
    seterror("[ REQUEST ] command '{}' failed\n",req_command);
    return GPKIH_FAIL;
  }

  // create crt
  if(system(crt_command.c_str())){
    // fail
    seterror("[ CERTIFICATE SIGNING ] command '{}' failed\n",crt_command);
    return GPKIH_FAIL;
  }

  // ca | sv | cl | key | req | certificate created
  // add to database and create inline config file
  if(db::entities::add(profile.name, entity)){
    return GPKIH_FAIL;
  } 
 
  // create inline config file
  std::vector<Entity> hahahah{entity};
  if(__create_inline_config(profile, config, hahahah, autoanswer, prompt)){
    return GPKIH_FAIL;
  }
  
  return GPKIH_OK;
}

template <typename ...T> int actions::build(T&& ...args){
  return build(std::forward<T>(args)...);
}