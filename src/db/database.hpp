#pragma once
#include "../config/config_management.hpp"
#include <map>
#include <vector>


namespace gpkih::db::profiles 
{
  inline std::map<std::string, Profile> existing_profiles{};
  inline str dbheaders = "name,source,ca,total_servers,total_clients";
  
  inline str dbpath{}; // set by main()

  inline int initialized = 0;
  extern int sync();  // overwrites profiles.csv with the profiles in existing_profiles while
  extern size_t initialize(strview path);
  extern int populate_from_entry(str &entry, Profile *profile);
  extern int populate_from_entry(str &entry, std::vector<str> &fields);
  extern int exists(strview profile_name);
  extern int add(Profile *profile);
  extern int remove(std::vector<str> &profiles);
  extern int remove_all();
  extern int load(strview profile_name, Profile &pinfo);
  extern int get_entities(str profile, std::vector<Entity> &buff);
  extern Profile *const load(strview profile_name);
} // namespace gpkih::db::profiles

namespace gpkih::db::entities {
  static inline str suffix = "_entities";
  static inline str ext = ".csv";
  static inline str _dbpath(str &profile) {
    return gpkih::db::profiles::dbpath + profile + suffix + ext;
  }

  inline str dbheaders =
    "common_name,type,serial,country,state,"
    "location,organisation,email,key_path,csr_path,crt_path";

  inline int initialized = 0;
/*  */
  extern int populate_from_entry(str &entry, Entity &entity);
  extern int populate_from_entry(str &entry, std::vector<str> &fields);
  extern int populate_from_entry(str &profile_name, str &entry, str &cn, Entity &buff);

  extern int initialize(str profile);
  extern int exists(strview profile_name, strview common_name);
  extern int add(str &profile_name, Entity &entity);
  extern int del(str &profile, strview cn);
  extern int load(str &profile, strview common_name, Entity &entity_buff);
} // namespace gpkih::db::entities