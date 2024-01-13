#pragma once
#include "structs.hpp"
#include <filesystem>

#ifdef _WIN32
#define CURRENT_PATH std::filesystem::current_path().string()
#define SLASH std::string("\\")
#else
/* LINUX STUFF */
#define CURRENT_PATH std::string(std::filesystem::current_path())
#define SLASH std::string("/")
#endif

namespace gpki::globals {
static inline std::string srcdir = CURRENT_PATH;
static inline std::string configdir = CURRENT_PATH + SLASH + ".." + SLASH + "config";
static inline std::string dbpath = CURRENT_PATH + SLASH + "gpki.db";
static inline Profile __current_profile{};
#define PROFILE &gpki::globals::__current_profile

static inline Entity __current_entity{};
#define ENTITY &gpki::globals::__current_entity

static inline int prompt = 1;
} // namespace gpki::globals
