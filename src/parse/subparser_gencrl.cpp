#include "subparser.hpp"
using namespace gpki;
int subparsers::gencrl(std::vector<std::string> opts){
  // ./gpki gencrl <profile> [subopts]
  if(opts.empty()){
    PERROR("profile must be given\n");
    PINFO("try gpki help gencrl\n");
    return -1;
  }
  strview profilename = opts[0];
  subopts::gencrl params;
  if(db::profiles::load(profilename,params.profile)){
    PERROR("profile '{}' doesn't exist\n");
    return -1;
  }
  return actions::gencrl(params);
}
