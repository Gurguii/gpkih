#pragma once
#include "structs.hpp"
#include <filesystem>
#include <iostream>


#ifdef _WIN32
#define CURRENT_PATH std::filesystem::current_path().string()
#define SLASH std::string("\\")
#else
/* LINUX STUFF */
#define CURRENT_PATH std::string(std::filesystem::current_path())
#define SLASH std::string("/")
#endif

#define FIELD_DELIMITER_s ":"
#define FIELD_DELIMITER_c ':'

inline void UNKNOWN_OPTION_MSG(std::string_view opt){ 
  std::cout << "[!] unknown option '" << opt << "'" << std::endl;
}

namespace gpki::globals {
static inline std::string srcdir = CURRENT_PATH;
static inline std::string configdir = CURRENT_PATH + SLASH + ".." + SLASH + "config";
static inline std::string user_used_command;
static inline int prompt = 1;
} // namespace gpki::globals
