#include "subparser.hpp"

using namespace gpki;
int subparsers::profile(std::vector<std::string> opts) { 
  for(int i = 0; i < opts.size(); ++i){
    std::string_view opt = opts[i];
    if(opt == "add"){
      gpki::modes::profile::create();
    }
  }
  return 0;
};
