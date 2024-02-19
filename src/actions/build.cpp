#include "actions.hpp"
using namespace gpkih;

// Not allowed in common_names


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

  str csr_command = std::move(fmt::format("openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc",
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
  
  str crt_command = std::move(fmt::format("openssl ca -config {} -in {} -out {} -subj '{}' -extfile {} -days +{}",
    profile.gopenssl(),
    entity.csr_path,
    entity.crt_path,
    subject.oneliner(),
    x509_extensions_file_path,
    days
  ));

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

  str command = fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj '{}' -set_serial '{}' -noenc",
    profile.gopenssl(),
    ca_crt_path,
    ca_key_path,
    entity.subject.oneliner(),
    entity.serial
  );

  return std::move(command);
};

static inline int _create_config(Profile &profile,
                                 std::vector<Entity> &entities,
                                 int do_inline_file = 1) {
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
    str entity_key_path = profile_dir_key + entity.subject.cn;
    str entity_crt_path = profile_dir_crt + entity.subject.cn;
    auto outdir = fmt::format("{}{}packs{}{}", profile.source, SLASH, SLASH,
                              entity.subject.cn);
    if (create_output_path(outdir)) {
      return GPKIH_FAIL;
    };
    str outpath;
    if (do_inline_file) {
      outpath = fmt::format("{}{}inline_{}.{}", outdir, SLASH,
                            entity.subject.cn, VPN_CONFIG_EXTENSION);
    } else {
      outpath = fmt::format("{}{}pack_{}", outdir, SLASH, entity.subject.cn);
      if (create_output_path(outpath)) {
        return GPKIH_FAIL;
      };
      // Copy files to pack dir
      fs::copy(entity.key_path, outpath);
      fs::copy(entity.crt_path, outpath);
      fs::copy(ca_crt_path, outpath);
      // Dump config file
      // GpkihConfig::dump(fmt::format("{}{}{}.{}",outpath,SLASH,entity.subject.cn,VPN_CONFIG_EXTENSION),entity.type);
      return GPKIH_OK;
    }
    // if(GpkihConfig::dump(outpath,entity.type)){
    //   PWARN("couldn't create config file for entity with CN '{}'\n",
    //   entity.subject.cn); return GPKIH_FAIL;
    // }
    /* At this point the common & (client|server) config has already been added
     * to the file so just check if */
    int _e_crt = fs::file_size(entity.crt_path);
    int _e_key = fs::file_size(entity.key_path);
    str entity_crt_str("\0", _e_crt);
    str entity_key_str("\0", _e_key);
    std::ifstream(entity.crt_path)
        .read(&entity_crt_str[0], entity_crt_str.size());
    std::ifstream(entity.key_path)
        .read(&entity_key_str[0], entity_key_str.size());
    if (entity_crt_str.empty()) {
      seterror("couldn't load entity certificate '{}'\n", entity.crt_path);
      return GPKIH_FAIL;
    }
    if (entity_key_str.empty()) {
      seterror("couldn't load entity key '{}'\n", entity.key_path);
      return GPKIH_FAIL;
    }
    std::ofstream file(outpath, std::ios::app);
    if (!file.is_open()) {
      seterror("couldn't open file '{}'\n", outpath);
      return GPKIH_FAIL;
    }
    // Append the inlined values
    file << "<ca>" << EOL << ca_crt_str << "</ca>" << EOL;
    file << "<cert>" << EOL << entity_crt_str << "</cert>" << EOL;
    file << "<key>" << EOL << entity_key_str << "</key>" << EOL;
    // TODO - check if tls-auth property is set and append it  too
  }
  return GPKIH_OK;
}
static inline int _create_config(str &profile_name,
                                 std::vector<str> &common_names,
                                 int do_inline_file = 1) {
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
  return _create_config(profile, entities, do_inline_file);
}

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
  serial_file.seekg(SEEK_SET);
  if(nserial.size() == 1){
    serial_file << "0";
  }
  std::cout << "new serial: " << nserial << "\n";
  std::cout << "adding it to '" << filepath << "'\n";
  serial_file << nserial;

  // Add to database
  if(db::entities::add(profile.name,entity)){
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

/* BUILD CA-SERVER-CLIENT CERTIFICATES */
int actions::build(Profile &profile, ProfileConfig &config, Entity &entity){
  ConfigMap &pkiconf = config._get(CONFIG_PKI);
  const auto [req_command, crt_command] = _server_client_build_commands(profile, pkiconf, entity);
  // create key + csr
  std::cout << req_command << "\n" << crt_command << "\n";
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

  // create inline config file
  std::vector<Entity> hahahah{entity};
  if(_create_config(profile, hahahah)){
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

template <typename ...T> int actions::build(T&& ...args){
  return build(std::forward<T>(args)...);
}