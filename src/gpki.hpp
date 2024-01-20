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

enum class ENTITY_TYPE
{
  none,
  ca,
  sv,
  cl
};
const auto entity_type_str = [](ENTITY_TYPE type){
    return (type == ENTITY_TYPE::ca ? "ca" : (type == ENTITY_TYPE::sv ? "server" : "client"));
};

namespace gpki::globals {
static inline std::string srcdir = CURRENT_PATH;
static inline std::string configdir = CURRENT_PATH + SLASH + ".." + SLASH + "config";
static inline std::string user_used_command;
static inline int prompt = 1;
} // namespace gpki::globals
