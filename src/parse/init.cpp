#include "parser.hpp"

using namespace gpki;

int parsers::init(std::vector<std::string> opts){
  opts.push_back("\0");
  subopts::init params;
  for(int i = 0; i < opts.size() -1 ; ++i){
    std::string_view opt = opts[i]; 
    if(opt == "-n" || opt == "--name"){
      params.profile_name = opts[++i];
    }else if(opt == "-s" || opt == "--source"){
      params.profile_source = opts[++i];
    }else{
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  actions::init(params);
  return 0;
}
