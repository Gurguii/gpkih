#pragma once
#include "structs.hpp"
#include <filesystem>

template <typename T> int IS_ABSOLUT_PATH(T path) {
#ifdef _WIN32
  return std::isalpha(path[0]);
#else
  return (path[0] == '/');
#endif
};

#ifdef _WIN32

#else
/* LINUX STUFF */
#define CURRENT_PATH std::string(std::filesystem::current_path())
#define SLASH std::string("/")
#endif

namespace gpki::globals {
static inline std::string srcdir = CURRENT_PATH;
static inline std::string configdir = CURRENT_PATH + SLASH + "config";
static inline std::string dbpath = CURRENT_PATH + SLASH + "gpki.db";
static inline Profile __current_profile{};
#define PROFILE &gpki::globals::__current_profile

static inline Entity __current_entity{};
#define ENTITY &gpki::globals::__current_entity

static inline int prompt = 1;
} // namespace gpki::globals
