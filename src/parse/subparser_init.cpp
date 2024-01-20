#pragma once
#include "subparser.hpp"
using namespace gpki;
// SYNTAX ./gpki init [subopts]
int subparsers::init(std::vector<std::string> opts){
  // No options added yet, straight call the init function
  opts.push_back("\0");
  subopts::init params;
  for(int i = 0; i < opts.size() -1 ; ++i){
    std::string_view opt = opts[i]; 
    if(opt == "-n" || opt == "--name"){
      params.profile_name = opts[++i];
    }else if(opt == "-s" || opt == "--source"){
      params.profile_source = opts[++i];
    }
  }
  actions::init(&params);
  return 0;
}
