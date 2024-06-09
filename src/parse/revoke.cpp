#include "parser.hpp"

// SYNTAX ./gpki revoke <profile> <cn1,cn2 ... cnX> [subopts]
using namespace gpkih;
int parsers::revoke(std::vector<std::string> &opts) {
  if (opts.size() == 0) {
    PERROR("profile name must be given\n");
    PHINT("try gpki help revoke\n");
    return -1;
  }
  
  std::string_view profilename = opts[0];
  Profile profile;

  std::vector<str> cnToRevoke;
  std::vector<str> serialsToRevoke;

  if (db::profiles::load(profilename, profile)) {
    PERROR("Profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  };
  
  opts.erase(opts.begin());
  opts.push_back("\0");

  if (opts.size() == 1) {
    PERROR("Missing serial | common_name\n");
    PHINT("Try gpki help revoke\n");
    return GPKIH_FAIL;
  }

  /* Revoke subopts */
  for(int i = 0; i < opts.size(); ++i){
    std::string_view opt = opts[i];
    if(opt == "-cn" || opt == "--common-name"){
      str cn;
      sstream ss(opts[++i]);
      while(getline(ss,cn,',')){
        cnToRevoke.emplace_back(cn);
      }  
    }else if(opt == "-s" || opt == "--serial"){
      str serial;
      sstream ss(opts[++i]);
      while(getline(ss,serial,',')){
        serialsToRevoke.emplace_back(serial);
      }
    }
  }

  if(cnToRevoke.empty()){
    PERROR("No common names provided, did you miss -cn | --common-name?\n");
    return GPKIH_FAIL;
  }

  opts.erase(opts.begin(), opts.begin() + 2);

  return actions::revoke(profile, cnToRevoke, serialsToRevoke);
}