#include "actions.hpp"
using namespace gpkih;

// Not allowed in common_names
str badchars = "~`!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?\t\n\r";

static inline void _get_and_set_prop(std::string &st) {
  std::string input;
  std::getline(std::cin, input);
  if (!input.empty()) {
    st = std::move(input);
  }
}

static inline int _prompt_for_subject(strview profile_name, Subject &buffer)
{
  str input{};
  PROMPT("Country Name (2 letter code) [" + buffer.country + "]: ");
  std::getline(std::cin, input);
  if (!input.empty() && input.size() == 2) {
    buffer.country = input;
  }
  // Set state name
  PROMPT("State or Province Name (full name) [" + buffer.state + "]: ");
  _get_and_set_prop(buffer.state);
  // Set location
  PROMPT("Locality Name [" + buffer.location + "]: ");
  _get_and_set_prop(buffer.location);
  // Set organisation
  PROMPT("Organisation Name [" + buffer.organisation + "]: ");
  _get_and_set_prop(buffer.organisation);
  // *MANDATORY Set common name
  input.assign("");
  // PROMPT("Common Name: ");
  // std::getline(std::cin, input);
  int keepgoing = 1;
  while (keepgoing) {
    PROMPT("Common Name: ", RED);
    std::getline(std::cin, input);
    if (input.empty()) {
      PWARN("common name can't be empty\n");
      continue;
    } else {
      keepgoing = 0;
      for (const char &c : input) {
        if (badchars.find(c) != -1) {
          // found bad char
          PWARN("found unaccepted char '{}'\nplease avoid using any of these "
                "'{}'\n",
                c, badchars);
          keepgoing = 1;
          break;
        }
      }
    }
  }
  buffer.cn = input;
  // Set email
  PROMPT("Email Address: ");
  _get_and_set_prop(buffer.email);

  if (db::entities::exists(profile_name, buffer.cn)) {
    seterror("Entity with CN '{}' already exists in profile '{}'\n",
             buffer.cn, profile_name);
    return GPKIH_FAIL;
  }
  return GPKIH_OK;
}

static str _server_client_csr_command(Profile &profile, ConfigMap &pkiconf,
                                             Subject &subject) {
  // openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} -keyform {} -noenc
    
  return fmt::format(
      "openssl req -newkey {}:{} -out {} -keyout {} -subj '{}' -outform {} "
      "-keyform {} -noenc", pkiconf["key"]["algorithm"],
      pkiconf["key"]["size"], profile.dir_req() + subject.cn + "-csr." + pkiconf["csr"]["creation_format"], profile.dir_key() + subject.cn + "-key." + pkiconf["key"]["creation_format"], subject.oneliner(),
      pkiconf["csr"]["creation_format"], pkiconf["key"]["format"]);
}
static str _server_client_crt_command(Profile &profile, ConfigMap &pkiconf,
                                             Entity &entity) {
  return fmt::format("openssl ca -config {} -in {} -out {} -subj '{}' -extfile "
                     "{}x509{}{} -notext -days +{}",
                     profile.gopenssl(), profile.dir_req() + entity.subject.cn + "-csr." + pkiconf["csr"]["creation_format"], profile.dir_crt() + entity.subject.cn + "-crt." + pkiconf["crt"]["format"],
                     entity.subject.oneliner(), CONF_DIRPATH, SLASH,
                     to_str(entity.type), pkiconf["crt"]["days"]);
}
static inline int _create_config(Profile &profile,
                                 std::vector<Entity> &entities,
                                 int _inline = 1) {
  str ca_crt_path = profile.ca_crt();

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
    if (_inline) {
      outpath = fmt::format("{}{}inline_{}.{}", outdir, SLASH,
                            entity.subject.cn, VPN_CONFIG_EXTENSION);
    } else {
      outpath = fmt::format("{}{}pack_{}", outdir, SLASH, entity.subject.cn);
      if (create_output_path(outpath)) {
        return GPKIH_FAIL;
      };
      // Copy files to pack dir
      fs::copy(entity_key_path, outpath);
      fs::copy(entity_crt_path, outpath);
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
    int _e_crt = fs::file_size(entity_crt_path);
    int _e_key = fs::file_size(entity_key_path);
    str entity_crt_str("\0", _e_crt);
    str entity_key_str("\0", _e_key);
    std::ifstream(entity_crt_path)
        .read(&entity_crt_str[0], entity_crt_str.size());
    std::ifstream(entity_key_path)
        .read(&entity_key_str[0], entity_key_str.size());
    if (entity_crt_str.empty()) {
      seterror("couldn't load entity certificate '{}'\n", entity_crt_path);
      return GPKIH_FAIL;
    }
    if (entity_key_str.empty()) {
      seterror("couldn't load entity key '{}'\n", entity_key_path);
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
                                 int _inline = 1) {
  Profile profile;
  if (db::profiles::load(profile_name, profile)) {
    seterror("couldn't load profile '{}'\n", profile_name);
    return GPKIH_FAIL;
  }
  std::vector<Entity> entities;
  for (auto cn : common_names) {
    Entity e;
    if (db::entities::load(profile_name, cn, e)) {
      seterror("couldn't load entity with cn '{}' from profile '{}'\n", cn,
               profile_name);
      return GPKIH_FAIL;
    };
    entities.emplace_back(e);
  }
  return _create_config(profile, entities, _inline);
}
/* BUILD CA-SERVER-CLIENT CERTIFICATES */

int actions::build(Profile &profile, ProfileConfig &config, Entity &entity){
  
  PINFO("CALLING actions::build(Profile, ProfileConfig, Entity)\n");
  if(entity.subject.cn.empty()){
    // User didn't give common_name (mandatory) with cli opts
    // set default values loaded from pki.conf
    entity.subject = ProfileConfig::default_subject(config);  
    // prompt the user for subject info
    _prompt_for_subject(profile.name, entity.subject);
  }

  // [rem] entity already has a type and filled subject info
  // entity.subject.print();
  // std::cout << "oneliner: " << entity.subject.oneliner() << "\n";
  ConfigMap &pkiconf = config._get(CONFIG_PKI);
  str key_csr_command = _server_client_csr_command(profile, pkiconf, entity.subject); 
  str crt_command = _server_client_crt_command(profile, pkiconf, entity);
  fmt::print("== commands ==\nkey + csr -> {}\ncrt -> {}\n", key_csr_command, crt_command);
  return GPKIH_OK;
}

template <typename ...T> int actions::build(T&& ...args){
  return build(std::forward<T>(args)...);
}