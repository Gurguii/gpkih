#pragma once
#include "../gpki.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#ifdef __WIN32
#define EOL std::string("\r\n")
#else
#define EOL std::string("\n")
#endif

namespace gpki::db {} // namespace gpki::db

namespace gpki::db::profiles {
static inline std::string dbheaders = "id,name,source" + EOL;
static inline std::string dbpath = CURRENT_PATH + SLASH + "profiles.csv";
static int initialize();
static int populate_entry(std::string entry, Profile *profile);
static int exists(Profile *profile);
static int exists(std::string_view profile_name);
static int add(Profile *profile);
static int del(Profile *profile);

} // namespace gpki::db::profiles
namespace gpki::db::entities {
static inline std::string dbheaders =
    "profile_name,common_name,type,country,state,location,organisation,email,key_"
    "path,req_path,"
    "cert_path";
static inline std::string dbpath = CURRENT_PATH + SLASH + "entities.csv";
static int populate_entry(std::string entry, Entity *entity);
static int initialize();
static int exists(Entity *entity);
static int add(Entity *entity);
static int del(Entity *entity);
} // namespace gpki::db::entities
