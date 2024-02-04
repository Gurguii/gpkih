#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ostream>
#include <algorithm>
#include <cstdint>

#include "../gpki.hpp"
#include "../db/database.hpp"
#include "../utils/gpkih_util_funcs.hpp"
#include "../utils/vpn_config.hpp"
#include "../printing.hpp"
#include "../parse/subparser.hpp"

namespace gpki::subopts { 
// Generic params not related with any action
struct params{
  int prompt = 1;
};

struct init : params{
  std::string profile_name;
  std::string profile_source;
};

struct build : params{
  std::string key_size = "1024";
  std::string algorithm = "rsa";
  std::string key_format = "pem";
  std::string csr_crt_format = "pem";
  ENTITY_TYPE type;
  Profile profile;
};

struct revoke : params{
  std::string common_name;
  std::string reason = "not specified";
  Profile profile;
};

struct gencrl : params{
  /* No subopts */
  Profile profile;
};

struct list : params{
  std::vector<std::string> profiles;
  std::vector<std::string> entities;

  PROFILE_FIELDS pfields = P_ALL;
  ENTITY_FIELDS efields = E_ALL;
};

struct remove : params{
  str profile_name;
};

struct remove_all : params{
  /* No subopts */
};

struct create_pack : params{
  /* */
  Profile profile;
  std::vector<Entity> entities;
  int inline_outfile = 0;
  strview outdir;
};

// ./gpki set <profile> <prop>=<val> <prop>=<val>
struct get : params{
  Profile profile;
};

struct set : params{
  Profile profile;
};
} // namespace gpki::subopts ./gpki create-pack <profile> <cn1,cn2...cnX>


namespace gpki::actions
{
  int init(subopts::init &params);
  int build(subopts::build &params);
  int revoke(subopts::revoke &params);
  int gencrl(subopts::gencrl &params);
  int list(subopts::list &params);
  int create_pack(subopts::create_pack &params);
  
  /* Getting/setting vpn config properties from profile*/
  int get(subopts::get &params);
  int set(subopts::set &params);
  // remove() usable but not added here yet
}