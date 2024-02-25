#include "actions.hpp"

#include <algorithm>
using namespace gpkih;

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

static int _create_inline_config(Profile &profile,ProfileConfig &config,
                                 std::vector<Entity> &entities) {
  str ca_crt_path = std::move(profile.ca_crt());

  if (!fs::exists(ca_crt_path)) {
    seterror("ca cert path '{}' doesn't exist\n", ca_crt_path);
    return GPKIH_FAIL;
  }
  // Load CA certificate
  int _ca_filesize = fs::file_size(ca_crt_path);
  str ca_crt_str("\0", _ca_filesize);
  std::ifstream(ca_crt_path).read(&ca_crt_str[0], ca_crt_str.size());
  if (ca_crt_str.empty()) {
    seterror("couldn't load CA certificate '{}'\n", ca_crt_path);
    return GPKIH_FAIL;
  }

  str profile_dir_key = std::move(profile.dir_key());
  str profile_dir_crt = std::move(profile.dir_crt());
  
  // CA certificate loaded
  for (auto &entity : entities) {
    str entity_key_path = std::move(profile_dir_key + entity.subject.cn);
    str entity_crt_path = std::move(profile_dir_crt + entity.subject.cn);
    auto outdir = fmt::format("{}{}packs{}{}", profile.source, SLASH, SLASH,
                              entity.subject.cn);

    if (create_output_path(outdir, 1)) {
      return GPKIH_FAIL;
    };

    str outpath = std::move(fmt::format("{}{}pack_{}", outdir, SLASH, entity.subject.cn));
    if (create_output_path(outpath, 0)) {
      return GPKIH_FAIL;
    };

    // Add the generic vpn configuration based
    // on entity type (client|server) - this config
    // is loaded from profile's openvpn.conf and might
    // get overriden by command
    if(config.dump_vpn_conf(outpath, entity.type) == false){
      return GPKIH_FAIL;
    };

    // Load entity certificate + key;
    const auto [entity_crt_str, entity_key_str] = load_entity_files(entity);

    std::ofstream file(outpath, std::ios::app);
    if (!file.is_open()) {
      seterror("couldn't open file '{}'\n", outpath);
      return F_NOOPEN;
    }

    // Append the inlined values
    file << "<ca>" << EOL << ca_crt_str << "</ca>" << EOL;
    file << "<cert>" << EOL << entity_crt_str << "</cert>" << EOL;
    file << "<key>" << EOL << entity_key_str << "</key>" << EOL;
    
    file.close();
  }
  return GPKIH_OK;
}
static inline int _create_config(str &profile_name,ProfileConfig &config,
                                 std::vector<str> &common_names) {
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
  return _create_inline_config(profile, config, entities);
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
  // to  
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
  const auto [req_command, crt_command] = _server_client_build_commands(profile, pkiconf, entity);
  // create key + csr
  fmt::print("{}\n{}\n", req_command, crt_command);

  if(system(req_command.c_str())){
    // fail
    seterror("command '{}' failed\n",req_command);
    return GPKIH_FAIL;
  }

  // create crt
  if(system(crt_command.c_str())){
    // fail
    seterror("command '{}' failed\n",crt_command);
    return GPKIH_FAIL;
  }

  // ca | sv | cl | key | req | certificate created
  // add to database and create inline config file
  if(db::entities::add(profile.name, entity)){
    return GPKIH_FAIL;
  } 
  return GPKIH_OK;
  // Add log
  gpkih::Logger::add(L_INFO, "added profile '{}'", profile.name);

 
  // create inline config file
  std::vector<Entity> hahahah{entity};
  if(_create_inline_config(profile, config, hahahah)){
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

template <typename ...T> int actions::build(T&& ...args){
  return build(std::forward<T>(args)...);
}