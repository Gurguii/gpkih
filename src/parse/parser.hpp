#pragma once
#include "../actions/actions.hpp"
#include "../config_management.hpp"
#include "../help/help.hpp"
#include "../printing.hpp"
#include "../structs.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <map>
#include <string>
#include <vector>

static inline int check_and_populate_profile(strview profilename, Profile &buffer,
                               std::vector<str> &opts) {
  if (db::profiles::load(profilename, buffer)) {
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
static int parse(int argc, const char **_args);
// action parsers
static int init(std::vector<str> opts);
static int build(std::vector<str> opts);
static int revoke(std::vector<str> opts);
static int gencrl(std::vector<str> opts);
static int list(std::vector<str> opts);
static int remove(std::vector<str> opts);
static int config(std::vector<str> opts);
// unimplemented
static int genkey(std::vector<str> opts);
} // namespace gpkih::parsers

static inline std::unordered_map<str, int (*)(std::vector<str>)> ACTION_PARSERS{
    {"init", gpkih::parsers::init},     {"list", gpkih::parsers::list},
    {"build", gpkih::parsers::build},   {"revoke", gpkih::parsers::revoke},
    {"gencrl", gpkih::parsers::gencrl}, {"remove", gpkih::parsers::remove},
    {"config", gpkih::parsers::config}, {"genkey", gpkih::parsers::genkey}};