#pragma once
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "../db/database.hpp"
#include "../gpki.hpp"
#include "../printing.hpp"
#include "../utils/gpkih_util_funcs.hpp"
#include "../utils/vpn_config.hpp"

namespace gpki::subopts {
// Generic params not related with any particular action
struct params {
  static inline int prompt = 1;
  static inline int autoanswer_yes = 0;
};

struct init : params {
  std::string profile_name;
  std::string profile_source;
};

struct build : params {
  static inline std::string key_size = "2048";
  static inline std::string algorithm = "rsa";
  static inline std::string key_format = "pem";
  static inline std::string csr_crt_format = "pem";
  static inline std::string days = "60";
  ENTITY_TYPE type;
  Profile* profile = nullptr;
};

struct revoke : params {
  std::vector<str> common_name;
  std::string reason = "not specified";
  Profile profile;
};

struct gencrl : params {
  /* No subopts */
  Profile profile;
};

struct list : params {
  std::vector<std::string> profiles;
  std::vector<std::string> entities;

  PROFILE_FIELDS pfields = P_ALL;
  ENTITY_FIELDS efields = E_ALL;
};

struct remove : params {
  std::vector<str> profiles;
  int all = 0;
};

// ./gpki set <profile> <prop>=<val> <prop>=<val>
struct get : params {
  ui64 properties;
  std::vector<str> cl_properties;
  std::vector<str> sv_properties;
  std::vector<str> common_properties;
  Profile profile;
};

struct set : params {
  Profile profile;
};
} // namespace gpki::subopts

namespace gpki::actions {
int init(subopts::init &params);
int build(subopts::build &params);
int revoke(subopts::revoke &params);
int gencrl(subopts::gencrl &params);
int list(subopts::list &params);
int remove(subopts::remove &params);
/* Getting/setting vpn|pki config properties from profile*/
int get(subopts::get &params);
int set(subopts::set &params);
} // namespace gpki::actions
