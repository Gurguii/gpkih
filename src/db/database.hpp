#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <unordered_map>
#include <vector>

#include "../gpki.hpp"
#include "../printing.hpp"
#include "../structs.hpp"
#include "../config_management.hpp"

namespace gpkih::db::profiles {
  inline std::map<std::string, Profile> existing_profiles{};
  inline str dbheaders = "name,source,ca,total_servers,total_clients";
  inline str dbpath = DB_DIRPATH + "profiles.csv";
  inline int initialized = 0;
  int sync(); // overwrites profiles.csv with the profiles in existing_profiles while
  int initialize();
  int populate_from_entry(str &entry, Profile *profile);
  int populate_from_entry(str &entry, std::vector<str> &fields);
  int exists(strview profile_name);
  int add(Profile *profile);
  int remove(std::vector<str> &profiles);
  int remove_all();
  int load(strview profile_name, Profile &pinfo);
  int get_entities(str profile, std::vector<Entity> &buff);
  Profile *const load(strview profile_name);
} // namespace gpkih::db::profiles

namespace gpkih::db::entities {
  inline str suffix = "_entities";
  inline str ext = ".csv";
  inline str _dbpath(str &profile) {
  return DB_DIRPATH + profile + suffix + ext;
}

  inline str dbheaders =
    "common_name,type,serial,country,state,"
    "location,organisation,email,key_path,csr_path,crt_path";

  inline int initialized = 0;
/*  */
  int populate_from_entry(str &entry, Entity &entity);
  int populate_from_entry(str &entry, std::vector<str> &fields);
  int populate_from_entry(str &profile_name, str &entry, str &cn, Entity &buff);

  int initialize(str profile);
  int exists(strview profile_name, strview common_name);
  int add(str &profile_name, Entity &entity);
  int del(str &profile, strview cn);
  int load(str &profile, strview common_name, Entity &entity_buff);
} // namespace gpkih::db::entities
