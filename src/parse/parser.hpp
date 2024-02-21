#pragma once
#include "../db/database.hpp"
#include "../actions/actions.hpp"
#include "../config/config_management.hpp"
#include "../structs.hpp"
#include "../help/help.hpp"
#include "../printing.hpp"


inline int check_and_populate_profile(strview profilename, gpkih::Profile &buffer,
                               std::vector<str> &opts) {
  if (gpkih::db::profiles::load(profilename, buffer)) {
    // profile does not exist
    seterror("Profile '{}' doesn't exist\n", profilename);
    return GPKIH_FAIL;
  }
  // delete profile name from vector
  opts.erase(opts.begin());
  return GPKIH_OK;
};

namespace gpkih::parsers {
// main parser
int parse(int argc, const char **_args);
// action parsers
int init(std::vector<str> opts);
int build(std::vector<str> opts);
int revoke(std::vector<str> opts);
int gencrl(std::vector<str> opts);
int list(std::vector<str> opts);
int remove(std::vector<str> opts);
int config(std::vector<str> opts);
// unimplemented
int genkey(std::vector<str> opts);
} // namespace gpkih::parsers

inline std::unordered_map<str, int (*)(std::vector<str>)> ACTION_PARSERS{
    {"init", gpkih::parsers::init},     {"list", gpkih::parsers::list},
    {"build", gpkih::parsers::build},   {"revoke", gpkih::parsers::revoke},
    {"gencrl", gpkih::parsers::gencrl}, {"remove", gpkih::parsers::remove},
    {"config", gpkih::parsers::config}, {"genkey", gpkih::parsers::genkey}};