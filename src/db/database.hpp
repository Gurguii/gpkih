#pragma once
#include "../gpki.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <optional>

#ifdef __WIN32
#define EOL std::string("\r\n")
#else
#define EOL std::string("\n")
#endif


#define CSV_DELIMITER_s ","
#define CSV_DELIMITER_c CSV_DELIMITER_s[0]


namespace gpki::db::profiles {
static inline std::string dbheaders = "name,source";
static inline std::string dbpath = CURRENT_PATH + SLASH + "profiles.csv";
static int initialize();
static int populate_entry(std::string entry, Profile *profile);
static std::optional<Profile> get(std::string_view profile_name);
static int exists(Profile *profile);
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
static inline std::string dbpath = CURRENT_PATH + SLASH + "entities.csv";
static int populate_entry(std::string &entry, Entity *entity);
static int initialize();
static int exists(Entity *entity);
static int add(Entity *entity);
static int del(Entity *entity);
} // namespace gpki::db::entities
namespace gpki::db {
    static inline int initialize_dbs(){
        return (profiles::initialize() | entities::initialize());
};
} // namespace gpki::db
