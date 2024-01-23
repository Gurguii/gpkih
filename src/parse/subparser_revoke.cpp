#include "subparser.hpp"

// SYNTAX ./gpki revoke <profile> [subopts]
using namespace gpki;
int subparsers::revoke(Profile *profile, std::vector<std::string> opts){
  if(opts.size() == 0){
    help::revoke::usage();
    return -1;
  }
  opts.push_back("\0");
  subopts::revoke params;
  for(int i = 0; i < opts.size() -1; ++i){
    std::string_view opt = opts[i];
    if(opt == "-cn"){
      params.common_name = opts[++i];
    }else if(opt == "-reason"){
      params.reason = opts[++i];
    }else{
      std::cout << "option '" << opt << "' doesn't exist\n";
    }
  }
  if(params.common_name.empty()){
    std::cout << "[error] Please specify a common name | -cn <name>\n";
    return -1;
  }
  return actions::revoke(profile,&params);
}
