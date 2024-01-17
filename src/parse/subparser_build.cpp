#include "subparser.hpp"

// SYNTAX : ./gpki build <profile> [subopts]
using namespace gpki;
using Parameters = modes::build::build_params;

int subparsers::build(std::vector<std::string> opts) { 
  std::string_view profile_name = opts[0];
  int (*build_action)(Profile *profile, Entity *entity, Parameters *params) = nullptr;
  
  std::optional<Profile> profile = db::profiles::get(profile_name);
  Entity entity;
  Parameters params;

  if(!profile.has_value()){
    std::cout << "Profile '" << profile_name << "' doesn't exist\n";
    help::build::usage();
    return -1;
  }
  
  opts.erase(opts.begin(),opts.begin()+1);
  // to avoid std::out_or_range exception when doing opts[++i]
  opts.push_back("\0");
  
  // override default build params with user arguments and set
  // right function to be executed
  for(int i = 0; i < opts.size()-1; ++i){
    std::string_view opt = opts[i];
    std::cout << "opt -> " << opt << "\n";
    if(opt == "-ca"){
      entity.type = "ca";
      build_action = modes::build::ca;
    }else if(opt == "-sv" || opt == "-server"){
      entity.type = "server";
      build_action = modes::build::server;
    }else if(opt == "-cl" || opt == "-client"){
      entity.type = "client";
      build_action = modes::build::client;
    }else if(opt == "-keysize" || opt == "--keysize"){
      params.key_size = opts[++i];
    }else if(opt == "-keyformat"){
      params.key_format = opts[++i];
    }else if(opt == "-outformat"){
      params.csr_crt_format = opts[++i];
    }else{
      std::cout << "[!] unknown option '" << opt << "'\n";
    }
  }

  if(build_action == nullptr){
    help::build::usage();
    return -1;
  }

  // Populate entity with the subject info
  modes::build::get_entity(&profile.value(),&entity,&params);
  
  entity.profile_name = profile.value().name;

  // Initialize entity csv 
  db::entities::initialize();
  
  return build_action(&profile.value(),&entity,&params);
}
