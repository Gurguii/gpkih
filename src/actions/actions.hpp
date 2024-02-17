#pragma once
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "../config_management.hpp"
#include "../db/database.hpp"
#include "../gpki.hpp"
#include "../printing.hpp"
#include "../utils/gpkih_util_funcs.hpp"
namespace gpkih::subopts {
// Generic params not related with any particular action
struct params {
  static inline int prompt = 1;
  static inline int autoanswer_yes = 0;
};

struct init : params {
  str profile_name;
  str profile_source;
};

struct build : params {
  ENTITY_TYPE type;             // entity type to create - ET_CA|ET_SV|ET_CL
  Profile *profile = nullptr;   // info about target profile
  gpkih::ProfileConfig *config; // used to retrieve default pki building params
                                // (keysize, days etc.) +
};

struct revoke : params {
  std::vector<str> common_name;
  str reason = "not specified";
  Profile profile;
};

struct gencrl : params {
  /* No subopts */
  Profile profile;
};

struct list : params {
  std::vector<str> profiles;
  std::vector<str> entities;

  PROFILE_FIELDS pfields = P_ALL;
  ENTITY_FIELDS efields = E_ALL;
};

struct remove : params {
  std::vector<str> profiles;
  int all = 0;
};

} // namespace gpkih::subopts

namespace gpkih::actions {
int init(subopts::init &params);
int build(subopts::build &params);
int revoke(subopts::revoke &params);
int gencrl(subopts::gencrl &params);
int list(subopts::list &params);
int remove(subopts::remove &params);
} // namespace gpkih::actions
