#include "parser.hpp"



// SYNTAX : ./gpki build <profile> [subopts]
using namespace gpki;
int parsers::build(std::vector<std::string> opts) {
  if(opts.empty()){
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }
  
  strview profilename = opts[0];
  Profile *profile;
  
  if((profile = db::profiles::load(profilename)) == nullptr){
    PERROR("profile '{}' doesn't exist\n", profilename);
    return -1;
  }

  // Load profile configuration
  ProfileConfig _conf(*profile);

  subopts::build params{
    .type = ET_NONE,
    .profile = profile
  };

  auto &type = params.type;

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
