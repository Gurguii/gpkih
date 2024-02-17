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
static inline std::map<std::string, Profile> existing_profiles{};
static inline str dbheaders = "name,source,ca,total_servers,total_clients";
static inline str dbpath = DB_DIRPATH + "profiles.csv";
static inline int initialized = 0;
static int
sync(); // overwrites profiles.csv with the profiles in existing_profiles while
static int initialize();
static int populate_from_entry(str &entry, Profile *profile);
static int populate_from_entry(str &entry, std::vector<str> &fields);
static int exists(strview profile_name);
static int add(Profile *profile);
static int remove(std::vector<str> &profiles);
static int remove_all();
static int load(strview profile_name, Profile &pinfo);
static int get_entities(str profile, std::vector<Entity> &buff);
/* TESTING */
static Profile *load(strview profile_name);
} // namespace gpkih::db::profiles

namespace gpkih::db::entities {
static inline str suffix = "_entities";
static inline str ext = ".csv";
static inline str _dbpath(str &profile) {
  return DB_DIRPATH + profile + suffix + ext;
}

static inline str dbheaders =
    "profile_name,common_name,type,serial,country,state,"
    "location,organisation,email,key_"
    "path,req_path,"
    "cert_path";

static inline int initialized = 0;
/*  */
static int populate_from_entry(str &entry, Entity &entity);
static int populate_from_entry(str &entry, std::vector<str> &fields);
static int populate_from_entry(str &profile, str &entry, str &cn, Entity &buff);

static int initialize(str profile);
static int exists(strview profile, strview common_name);
static int add(Entity &entity);
static int del(str &profile, strview cn);
static int load(str &profile, strview common_name, Entity &entity_buff);
} // namespace gpkih::db::entities
