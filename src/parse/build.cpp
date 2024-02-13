#include "parser.hpp"


int load_profile_config(Profile *profile){
  if(!fs::exists(profile->source)){
    return -1;
  }
  if(GpkihConfig::load(*profile)){
    return -1;
  };
  return 0;
}

// SYNTAX : ./gpki build <profile> [subopts]
using namespace gpki;
int parsers::build(std::vector<std::string> opts) {
  if(opts.empty()){
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }
   
  subopts::build params;
  strview profilename = opts[0];
  params.profile = db::profiles::load(profilename);
  if(params.profile == nullptr){
    PERROR("profile '{}' doesn't exist\n", profilename);
    return -1;
  }
  std::cout << "LOADING PROFILE CONFIGURATION - line 29 parse/build.cpp\n";
  // Load profile config
  Config::load(*params.profile);
  auto vpn_conf = *Config::get(CONFIG_VPN);
  auto pki_conf = *Config::get(CONFIG_PKI);
  auto gpkih_conf = *Config::get(CONFIG_GPKIH);
  for(auto item : {vpn_conf,pki_conf,gpkih_conf}){
    // config file
    for(auto kv : item){
      // section
      std::cout << kv.first << "\n";
      for(auto keyval : kv.second){
        // key - value 
        std::cout << "key: " << keyval.first << " val: " << keyval.second << "\n";
      }
    }
  }
  return -1;
  //load_profile_config(params.profile);
  //if(db::profiles::load(profilename,profile)){
  //  PERROR("profile '{}' doesn't exist\n",profilename);
  //  return 0;
  //}

  auto &type = params.type;
  type = ENTITY_TYPE::none;
  opts.erase(opts.begin());
  opts.push_back("\0");
  // override default build params with user arguments and set
  for(int i = 0; i < opts.size(); ++i){
    std::string_view opt = opts[i];
    if(opt == "-ca" || opt == "--ca"){
      type = ENTITY_TYPE::ca;
    }else if(opt == "-cl" || opt == "--client"){
      type = ENTITY_TYPE::client;
    }else if(opt == "-sv" || opt == "--server"){
      type = ENTITY_TYPE::server;
    }else if(opt == "-keysize" || opt == "--keysize"){
      params.key_size = opts[++i];
    }else if(opt == "-keyformat"){
      params.key_format = opts[++i];
    }else if(opt == "-outformat"){
      params.csr_crt_format = opts[++i];
    }else if(opt == "\0"){
      continue;
    }else{
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  if(type == ENTITY_TYPE::none){
    PERROR("please specify an entity type -[ca|sv|cl]\n");
    return -1;
  }
  return actions::build(params);
}
