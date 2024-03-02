#pragma once
#include <vector>
#include "../actions/actions.hpp" // database.hpp gpki.hpp structs.hpp

inline int check_and_populate_profile(strview profilename, gpkih::Profile &buffer,
                               std::vector<str> &opts) {
  if (gpkih::db::profiles::load(profilename, buffer)) {
    // profile does not exist
    return GPKIH_FAIL;
  }
  // delete profile name from vector
  opts.erase(opts.begin());
  return GPKIH_OK;
};

namespace gpkih::parsers 
{
// main parser
extern int parse(int argc, const char **_args);

// action parsers
extern int init(std::vector<str> opts);
extern int build(std::vector<str> opts);
extern int revoke(std::vector<str> opts);
extern int gencrl(std::vector<str> opts);
extern int list(std::vector<str> opts);
extern int remove(std::vector<str> opts);

// partially implemented
extern int set(std::vector<str> opts);

// unimplemented
extern int get(std::vector<str> opts);

int genkey(std::vector<str> opts);
} // namespace gpkih::parsers

inline std::unordered_map<str, int (*)(std::vector<str>)> ACTION_PARSERS{
    {"init", gpkih::parsers::init},     {"list", gpkih::parsers::list},
    {"build", gpkih::parsers::build},   {"revoke", gpkih::parsers::revoke},
    {"gencrl", gpkih::parsers::gencrl}, {"remove", gpkih::parsers::remove},
    {"genkey", gpkih::parsers::genkey}, {"get", gpkih::parsers::get}, {"set", gpkih::parsers::set} };