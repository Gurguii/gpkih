#include "parser.hpp"
#include <chrono>
#include <cstring>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <thread>

static inline std::string badchars = "~`!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?¿\t\n\r";
static inline std::string mail_badchars = "~`!#$%^&*()-_=+[{]}\\|;:'\",<>/?¿\t\n\r";

using namespace gpkih;

static inline size_t __get_and_set_prop(std::string &&prompt_msg, char *&default_val, char *&st, size_t* stlen, std::string &badcharlist = badchars, size_t max_st_len = 254) {
  PDEBUG(2,"__get_and_set_prop()");

  std::string input = PROMPT(prompt_msg);

  if (input.empty()) {
    CALLOCATE(st, stlen, default_val);
    return 0;
  }

  if(REMOVE_BADCHAR(input, badcharlist)){
    PWARN("badchars were removed\n");
  };

  if(input.size() <= max_st_len){
    CALLOCATE(st, stlen, input);
  }else{
    PWARN("max length '{}' exceeded\n", max_st_len);
  }

  return 0;
}

static inline size_t __load_serial(Profile *profile, Entity &entity){
  PDEBUG(2,"__load_serial()");
  CALLOCATE(serial_path, &serial_path_len, fmt::format("{}{}pki{}serial{}serial", profile->source, SLASH, SLASH, SLASH));

  if(!fs::exists(serial_path)){
    seterror("serial file for profile '{}' not found\n", profile->name);
    return GPKIH_FAIL;
  }

  std::ifstream file(serial_path);

  if(!file.is_open()){
    seterror("couldn't open file '{}'\n",serial_path);
    return GPKIH_FAIL;
  }
  std::string serial{};
  file >> serial;
  file.close();

  entity.serial = std::stoi(serial);

  PDEBUG(3, "loaded serial: %lu\n", entity.serial);

  return GPKIH_OK;
};

static inline int __prompt_for_subject(std::string_view profile_name, Subject &buffer, ProfileConfig &config, EntityManager &eman)
{
  PDEBUG(2,"__prompt_for_subject()");

  Subject defaults = config.default_subject();

  std::string input{};

  // Set country if not set already
  if(len(buffer.country) == 0){
    PROMPT(fmt::format("Country Name (2 letter code) [{}]",defaults.country));
    if (!input.empty() && input.size() == 2) {
      memcpy(buffer.country, input.c_str(), input.size() + 1);
    }else{
      memcpy(buffer.country, defaults.country, 2);
    }
  }

  // Set state name if not set already
  if(buffer.state == nullptr){
    __get_and_set_prop(fmt::format("State or Province Name (full name) [{}]",defaults.state),defaults.state,buffer.state,reinterpret_cast<size_t*>(&buffer.statelen));  
  }

  // Set location if not set already
  if(buffer.location == nullptr){
    __get_and_set_prop(fmt::format("Locality Name [{}]",defaults.location),defaults.location, buffer.location,reinterpret_cast<size_t*>(&buffer.locationlen));
  }
  
  // Set organisation if not set already
  if(buffer.organisation == nullptr){
    __get_and_set_prop(fmt::format("Organisation Name [{}]",defaults.organisation),defaults.organisation, buffer.organisation,reinterpret_cast<size_t*>(&buffer.organisationlen));
  }
  
  input.assign("");

  // *MANDATORY Set common name
  for(;;) {
    input = PROMPT("Common Name", false, RED);
    if (input.empty()) {
      // Common name can't be empty
      PWARN("common name can't be empty\n");
      continue;
    } else if (eman.exists(input)) {
      // Common name can't be duplicated
      PWARN("Entity with CN '{}' already exists in profile '{}'\n",
      input, profile_name);
      continue;
    }else if(HAS_BADCHAR(input, badchars)) {
      continue;
    }

    // Got proper cn
    CALLOCATE(buffer.cn, reinterpret_cast<size_t*>(&buffer.cnlen), input);
    input.assign("");
    break;
  }
  
  // Set email if not set already
  if(buffer.email == nullptr){
    __get_and_set_prop("Email Address",defaults.email, buffer.email, reinterpret_cast<size_t*>(&buffer.emaillen), mail_badchars);  
  }

  return GPKIH_OK;
}

static inline std::unordered_map<ENTITY_TYPE,int(*)(Profile&,ProfileConfig&,Entity&,EntityManager&)> build_functions
{
  {ET_CA,actions::build_ca},
  {ET_CL,actions::build},
  {ET_SV,actions::build}
};

using namespace gpkih;

int parsers::build(std::vector<std::string> &opts) {
  PDEBUG(1, "parsers::build()");

  // ./gpki build <profile> <ca|sv|cl> [subopts]
  if (opts.empty()) {
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }

  std::string_view profilename = opts[0];
  
  Profile *profile; // profile to build entity for
  Entity entity;  // entity info

  
  profile = db::profiles::get(profilename);
  if(profile == nullptr){
    seterror("Profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  }
  
  if(opts.size() == 1){
    PERROR("entity type must be specified - ca|sv|cl\n");
    return GPKIH_OK;
  }

  std::string_view etype = opts[1];
  if(entity_type_map.find(etype.data()) == entity_type_map.end()){
    seterror("invalid entity type '{}'",etype);
    return GPKIH_FAIL;
  }
  
  entity.type = entity_type_map[etype.data()];
  opts.erase(opts.begin(),opts.begin()+2);
  
  /* Profile configuration + Entity manager */
  ProfileConfig config(*profile);
  EntityManager eman(profile->name);

  if(!config.succesfully_loaded){
    return GPKIH_FAIL;
  }

  if((entity.type & ET_CL || entity.type & ET_SV) && profile->ca_created == false)
  {
    PWARN("CA must be created before creating server/client entities\n");
    PHINT("{}/gpkih build {} ca -cn MyCA\n",fs::current_path().c_str(), profile->name);
    return GPKIH_OK;
  }

  // Load next serial
  __load_serial(profile, entity);
  
  // override default build params with user arguments
  for (int i = 0; i < opts.size(); ++i) {
    std::string_view opt = opts[i];
    PDEBUG(3, "opt:{}",opt);
    if(opt == "-algo" || opt == "--algorithm"){
      // check its a valid algorithm
      config.set(CONFIG_PKI,"key","algorithm",std::move(opts[++i]));
    }
    else if(opt == "-keysize" || opt == "--keysize") {
      config.set(CONFIG_PKI,"key","size",std::move(opts[++i]));
    } else if (opt == "-keyformat") {
      config.set(CONFIG_PKI,"key","creation_format",std::move(opts[++i]));
    } else if (opt == "-outformat") {
      config.set(CONFIG_PKI,"csr","creation_format",std::move(opts[++i]));
    } else if(opt == "-cn" || opt == "--common-name"){
      CALLOCATE(entity.subject.cn, reinterpret_cast<size_t*>(&entity.subject.cnlen), opts[++i]);
      if(eman.exists(entity.subject.cn)){
        PWARN("entity '{}' already exists in profile '{}'\n", entity.subject.cn, profile->name);
        return GPKIH_FAIL;
      }
    } else if(opt == "-serial" || opt == "--serial"){
      // todo - check if desired serial exists in an existing entity - add function db::entities::exists(int serial)
      size_t serial = std::move(stoi(opts[++i]));
      if(eman.exists(serial)){
        PWARN("entity with serial '{}' already exists\n");
      }else{
        entity.serial = serial;  
      }
    } else if(opt == "-loc" || opt == "--location"){
      CALLOCATE(entity.subject.location, reinterpret_cast<size_t*>(&entity.subject.locationlen), opts[++i]);
    } else if(opt == "-co" || opt == "--country"){
      if(len(opts[++i].c_str()) == 2){
        memcpy(entity.subject.country,opts[i].c_str(),2); 
      }else{
        PWARN("country must be a 2 letter code, e.g ES,EN,DE,FR ... omitting user value '{}'\n",opts[i]);
      }
    }else if(opt == "-org" || opt == "--organisation"){
      CALLOCATE(entity.subject.organisation,reinterpret_cast<size_t*>(&entity.subject.organisationlen),opts[++i]);
    }else if(opt == "-st" || opt == "--state"){
      CALLOCATE(entity.subject.state, reinterpret_cast<size_t*>(&entity.subject.statelen), opts[++i]);
    }else if(opt == "-email" || opt == "--email"){
      CALLOCATE(entity.subject.email, reinterpret_cast<size_t*>(&entity.subject.email), opts[++i]);
    }else if(opt == "-pfx" || opt == "--pfx"){
      config.set(CONFIG_PKI, "output", "create_pfx","yes");
    }else if(opt == "-inline" || opt == "--inline"){
      config.set(CONFIG_PKI, "output", "create_inline", "yes");
    }else if(opt == "-days" || opt == "--days"){
      int d = std::strtol(&opts[++i][0],NULL,10);
      if(d <= 0){
        PWARN("days must be a positive integer, ignoring given value {:+d}\n", d);
        continue;
      }
      config.set(CONFIG_PKI, "crt", "days", opts[i]);
    }else if(opt == "\0"){
      continue;
    }else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }

  if(entity.subject.cn == nullptr){
    // User didn't give common_name (mandatory) with cli opts  
    // prompt the user for subject info
    __prompt_for_subject(profile->name, entity.subject, config, eman);
  }

  return build_functions[entity.type](*profile,config,entity,eman);
}