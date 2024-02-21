#include "parser.hpp"

str badchars = "~`!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?\t\n\r";
using namespace gpkih;

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
          PWARN("found unaccepted char '{}' - please avoid using any of these "
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

static inline std::unordered_map<ENTITY_TYPE,int(*)(Profile&,ProfileConfig&,Entity&)> build_functions
{
  {ET_CA,actions::build_ca},
  {ET_CL,actions::build},
  {ET_SV,actions::build}
};

using namespace gpkih;

int parsers::build(std::vector<std::string> opts) {
  // ./gpki build <profile> <ca|sv|cl> [subopts]
  if (opts.empty()) {
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }

  strview profilename = opts[0];
  
  Profile profile; // profile to build entity for
  Entity entity;  // entity info

  if(db::profiles::load(profilename, profile)){
    return GPKIH_FAIL;
  };

  if(opts.size() == 1){
    PERROR("entity type must be specified - ca|sv|cl\n");
    return GPKIH_OK;
  }

  strview etype = opts[1];
  if(entity_type_map.find(etype.data()) == entity_type_map.end()){
    seterror("invalid entity type '{}'",etype);
    return GPKIH_FAIL;
  }
  entity.type = entity_type_map[etype.data()];

  opts.erase(opts.begin(),opts.begin()+2);

  ProfileConfig config(profile);
  entity.subject = std::move(config.default_subject());

  if(!config.succesfully_loaded){
    return GPKIH_FAIL;
  }

  // e.g /profiles/pki/serial/serial
  // Load next serial
  str serial_path = profile.source + SLASH + "pki" + SLASH + "serial" + SLASH + "serial";
  if(!fs::exists(serial_path)){
    seterror("serial file for profile '{}' not found\n", profile.name);
    return F_NOEXIST;
  }
  std::ifstream file(serial_path);
  if(!file.is_open()){
    seterror("couldn't open file '{}'\n",serial_path);
    return F_NOOPEN;
  }
  file >> entity.serial;

  // override default build params with user arguments
  for (int i = 0; i < opts.size(); ++i) {
    strview opt = opts[i];
    if(opt == "-keysize" || opt == "--keysize") {
      config.set(CONFIG_PKI,"key","size",std::move(opts[++i]));
    } else if (opt == "-keyformat") {
      config.set(CONFIG_PKI,"key","creation_format",std::move(opts[++i]));
    } else if (opt == "-outformat") {
      config.set(CONFIG_PKI,"csr","creation_format",std::move(opts[++i]));
    } else if(opt == "-cn" || opt == "--common-name"){
      entity.subject.cn = std::move(opts[++i]);
    } else if(opt == "-serial" || opt == "--serial"){
      // todo - check if desired serial exists in an existing entity - add function db::entities::exists(int serial)
      entity.serial = std::move(opts[++i]);
    } else if(opt == "-loc" || opt == "--location"){
      entity.serial = std::move(opts[++i]);
    } else if(opt == "-co" || opt == "--country"){
      // todo - ensure its a 2 char
      entity.subject.country = std::move(opts[++i]);
    }else if(opt == "-org" || opt == "--organisation"){
      entity.subject.country = std::move(opts[++i]);
    }else if(opt == "-st" || opt == "--state"){
      entity.subject.state = std::move(opts[++i]);
    }else if(opt == "\0"){
      continue;
    }
    else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  if(entity.subject.cn.empty()){
    // User didn't give common_name (mandatory) with cli opts  
    // prompt the user for subject info
    _prompt_for_subject(profile.name, entity.subject);
  }

  return build_functions[entity.type](profile,config,entity);
}
