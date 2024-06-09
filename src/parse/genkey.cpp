#include "parser.hpp"
using namespace gpkih;

// ./gpki genkey <profile> [subopts]
// [subopts]
// -dh
// -tls

int parsers::genkey(std::vector<str> &opts) {
 
  if (opts.empty()) {
    PERROR("profile name must be given\n");
    PHINT("try ./gpki help genkey\n");
    return -1;
  }

  std::string_view profilename = opts[0];
  
  Profile profile;
  
  if (db::profiles::load(profilename, profile)) {
    PERROR("profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  }

  for(auto iter = opts.begin()+1; iter != opts.end(); ++iter){
    std::string_view opt = *iter;
    if(opt == "-dh" || opt == "--dh"){

    }else if(opt == "" || opt == ""){

    }else{
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }
  
  return GPKIH_OK;
}
