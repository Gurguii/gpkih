#pragma once
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <vector>

#include "../structs.hpp"
#include "../gpki.hpp"
#include "../printing.hpp"

namespace gpki::db::profiles {
static inline std::map<std::string, Profile> existing_profiles{};
static inline str dbheaders = "name,source";
static inline str dbpath = DBDIR + "profiles.csv";
static int sync(); // overwrites profiles.csv with the profiles in existing_profiles while checking that source dirs from existing_profiles indeed exist
static int initialize();
static int populate_from_entry(str &entry, Profile *profile);
static int populate_from_entry(str &entry, std::vector<str> &fields);
static int exists(strview profile_name);
static int add(Profile *profile);
static int remove(str &profile);
static int remove_all();
static int load(strview profile_name, Profile &pinfo);
static int get_entities(str profile,std::vector<Entity> &buff);
} // namespace gpki::db::profiles

namespace gpki::db::entities {
static inline str dbheaders = "profile_name,common_name,type,country,state,"
                              "location,organisation,email,key_"
                              "path,req_path,"
                              "cert_path";
static inline int initialized = 0;
static inline str dbpath;
static int populate_from_entry(str &entry, Entity *entity);
static int populate_from_entry(str &entry, std::vector<str> &fields);
static int initialize(str profile);
static int exists(strview profile, strview common_name);
static int add(Entity *entity);
static int del(strview profile, strview cn);
static int load(Profile *profile, Entity *entity_buff, strview common_name);
} // namespace gpki::db::entities

namespace gpki::db {
static inline int initialize_dbs(str profile) {
  return (profiles::initialize() | entities::initialize(profile));
};
} // namespace gpki::db
