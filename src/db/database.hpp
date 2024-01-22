#pragma once
#include "../gpki.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

#define CSV_DELIMITER_s ","
#define CSV_DELIMITER_c ','

namespace gpki::db::profiles {
static inline std::unordered_map<std::string,Profile> existing_profiles{};
static inline std::string dbheaders = "name,source";
static inline std::string dbpath = CURRENT_PATH + SLASH + "profiles.csv";
static int initialize();
static int populate_from_entry(std::string &entry, Profile *profile);
static int populate_from_entry(std::string &entry, std::vector<std::string> &fields);
static int exists(std::string_view profile_name);
static int add(Profile *profile);
static int del(Profile *profile);
static int load(std::string_view profile_name, Profile &pinfo);
} // namespace gpki::db::profiles

namespace gpki::db::entities {
static inline std::string dbheaders =
    "profile_name,common_name,type,country,state,location,organisation,email,key_"
    "path,req_path,"
    "cert_path";
static inline int initialized = 0;
static inline std::string dbpath = CURRENT_PATH + SLASH + "entities.csv";
static int populate_from_entry(std::string &entry, Entity *entity);
static int populate_from_entry(std::string &entry, std::vector<std::string> &fields);
static int initialize();
static int exists(std::string_view profile, std::string_view common_name);
static int add(Entity *entity);
static int del(std::string_view profile, std::string_view cn);
static int load(Profile *profile, Entity *entity_buff, std::string_view common_name);
} // namespace gpki::db::entities
namespace gpki::db {
    static inline int initialize_dbs(){
        return (profiles::initialize() | entities::initialize());
};
} // namespace gpki::db
