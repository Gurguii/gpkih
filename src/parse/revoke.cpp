#include "parser.hpp"

// SYNTAX ./gpki revoke <profile> <cn1,cn2 ... cnX> [subopts]
using namespace gpkih;
int parsers::revoke(std::vector<std::string> opts) {
  if (opts.size() == 0) {
    PERROR("profile name must be given\n");
    PHINT("try gpki help revoke\n");
    return -1;
  }
  subopts::revoke params;
  strview profilename = opts[0];
  if (db::profiles::load(profilename, params.profile)) {
    PERROR("profile '{}' doesn't exist\n", profilename);
    return -1;
  };
  if (opts.size() == 1) {
    PERROR("missing common name\n");
    PHINT("try gpki help revoke\n");
    return -1;
  }
  sstream ss(opts[1]);
  str cn;
  while (getline(ss, cn, ',')) {
    params.common_name.emplace_back(cn);
  }
  opts.erase(opts.begin(), opts.begin() + 2);
  opts.push_back("\0");
  for (int i = 0; i < opts.size() - 1; ++i) {
    std::string_view opt = opts[i];
    if (opt == "--reason") {
      params.reason = opts[++i];
    } else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  if (params.common_name.empty()) {
    PWARN("common name required\n");
    PHINT("try gpki help revoke");
    return -1;
  }
  return actions::revoke(params);
}
