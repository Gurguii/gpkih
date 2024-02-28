#include "parser.hpp"
using namespace gpkih;

// ./gpki genkey <profile> [subopts]
// [subopts]
// -dh
// -tls

int parsers::genkey(std::vector<str> opts) {
  if (opts.empty()) {
    PERROR("profile name must be given\n");
    PHINT("try ./gpki help genkey\n");
    return -1;
  }
  strview profilename = opts[0];
  Profile profile;
  if (db::profiles::load(profilename, profile)) {
    PERROR("profile '{}' doesn't exist\n", profilename);
    return -1;
  }
  return 0;
}
