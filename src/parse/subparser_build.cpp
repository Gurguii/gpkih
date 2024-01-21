#include "subparser.hpp"

// string -> "holasdasd"
// vector<string> lista{"hola","soy","asdas","sdgs","asda"}
// lista[0] -> "hola"
// lista[1] -> "soy"
// SYNTAX : ./gpki build <profile> [subopts]
using namespace gpki;
int subparsers::build(Profile *profile, std::vector<std::string> opts) { 
  Entity entity;
  subopts::build params;
  ENTITY_TYPE type = ENTITY_TYPE::none;
  opts.push_back("\0");
  // override default build params with user arguments and set
  // right function to be executed
  for(int i = 0; i < opts.size()-1; ++i){
    std::string_view opt = opts[i];
    std::cout << "opt -> " << opt << "\n";
    if(opt == "-ca" || opt == "--ca"){
      type = ENTITY_TYPE::ca;
    }else if(opt == "-cl" || opt == "--client"){
      type = ENTITY_TYPE::cl;
    }else if(opt == "-sv" || opt == "--server"){
      type = ENTITY_TYPE::sv;
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
  if(type == ENTITY_TYPE::none){
    std::cout << "[error] Please specify an entity type\n";
    std::cout << globals::user_used_command << " -[ca|sv|cl]\n";
    return -1;
  }
  return actions::build(profile,&params,type);
}
