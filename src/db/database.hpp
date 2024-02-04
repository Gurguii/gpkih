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
static int remove(std::vector<str> &profiles);
static int remove_all();
static int load(strview profile_name, Profile &pinfo);
static int get_entities(str profile,std::vector<Entity> &buff);
} // namespace gpki::db::profiles

namespace gpki::db::entities {
static inline str suffix = "_entities";
static inline str ext = ".csv";
static inline str _dbpath(str &profile){return DBDIR + profile + suffix + ext;}

static inline str dbheaders = "profile_name,common_name,type,serial,country,state,"
                              "location,organisation,email,key_"
                              "path,req_path,"
                              "cert_path";

static inline int initialized = 0;
/*  */
static int populate_from_entry(str &entry, Entity &entity);
static int populate_from_entry(str &entry, std::vector<str> &fields);
static int populate_from_entry(str &profile, str &entry,str &cn,Entity &buff);

static int initialize(str profile);
static int exists(str &profile, strview common_name);
static int add(Entity &entity);
static int del(str &profile, strview cn);
static int load(str &profile, strview common_name, Entity &entity_buff);
} // namespace gpki::db::entities
