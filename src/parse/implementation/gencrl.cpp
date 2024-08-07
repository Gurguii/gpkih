#include "../parser.hpp"

using namespace gpkih;
int parsers::gencrl(std::vector<std::string> &opts) {
  DEBUG(1, "parsers::gencrl()");

  // ./gpki gencrl <profile> [subopts]
  if (opts.empty()) {
    PERROR("profile must be given\n");
    PINFO("try gpki help gencrl\n");
    return -1;
  }
  std::string_view profilename = opts[0];
  Profile profile;
  if (db::profiles::load(profilename, profile)) {
    return GPKIH_FAIL;
  }
  return actions::gencrl(profile);
}
