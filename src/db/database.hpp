#pragma once
#include "../gpki.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace gpki::db::profiles {
static inline std::unordered_map<std::string, Profile> existing_profiles{};
static inline str dbheaders = "name,source";
static inline str dbpath = DBDIR + "profiles.csv";
static int initialize();
static int populate_from_entry(str &entry, Profile *profile);
static int populate_from_entry(str &entry, std::vector<str> &fields);
static int exists(strview profile_name);
static int add(Profile *profile);
static int del(Profile *profile);
static int load(strview profile_name, Profile &pinfo);
static int get_entities(str profile,
                        std::vector<std::vector<str>> &entities_fields_buff);
static int get_fields(str profile,
                      std::vector<str> &fields_buff);
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
