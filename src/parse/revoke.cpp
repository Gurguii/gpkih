#include "parser.hpp"

// SYNTAX ./gpki revoke <profile> <cn1,cn2 ... cnX> [subopts]
using namespace gpkih;
int parsers::revoke(std::vector<std::string> opts) {
  if (opts.size() == 0) {
    PERROR("profile name must be given\n");
    PHINT("try gpki help revoke\n");
    return -1;
  }
  
  strview profilename = opts[0];
  Profile profile;
  std::vector<str> common_names_to_revoke;
  std::vector<str> serials_to_revoke;
  str extra_reason;

  if (db::profiles::load(profilename, profile)) {
    return GPKIH_FAIL;
  };
  
  opts.erase(opts.begin());
  opts.push_back("\0");

  if (opts.size() == 1) {
    PERROR("missing serial | common_name\n");
    PHINT("try gpki help revoke\n");
    return -1;
  }

  /* Revoke subopts */
  for(int i = 0; i < opts.size(); ++i){
    strview opt = opts[i];
    if(opt == "-cn" | opt == "--common-name"){
      str cn;
      sstream ss(opts[++i]);
      while(getline(ss,cn,',')){
        common_names_to_revoke.emplace_back(cn);
      }  
    }else if(opt == "-s" | opt == "-serial"){
      str serial;
      sstream ss(opts[++i]);
      while(getline(ss,serial,',')){
        serials_to_revoke.emplace_back(serial);
      }
    }
  }

  opts.erase(opts.begin(), opts.begin() + 2);
  opts.push_back("\0");
  for (int i = 0; i < opts.size() - 1; ++i) {
    std::string_view opt = opts[i];
    if (opt == "--reason") {
      extra_reason = opts[++i];
    } else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }

  return actions::revoke(profile, common_names_to_revoke, serials_to_revoke);
}
