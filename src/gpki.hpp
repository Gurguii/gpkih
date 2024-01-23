#pragma once
#include "structs.hpp"
#include <filesystem>
#include <iostream>
#include <cstdlib>

// Custom typenames
using str = std::string;
using strview = std::string_view;
using ui8 = uint8_t;
using ui16 = uint16_t;

// Custom namespace names
namespace fs = std::filesystem;

#ifdef _WIN32
/* WINDOWS STUFF */
#include <windows.h>
static inline str CURRENT_PATH = fs::current_path().string();
static inline str SLASH = "\\"
static inline str BASEDIR = str(std::getenv("LOCALAPPDATA")) + "\\gpkih\\";
#else
/* LINUX STUFF */
#include <unistd.h>
#include <pwd.h>
static inline str CURRENT_PATH = fs::current_path();
static inline str SLASH = "/";
static inline str BASEDIR = str(std::getenv("HOME")) + "/.config/gpkih/"; 
#endif

static inline str DBDIR = BASEDIR + "db" + SLASH;
static inline str CONFDIR = BASEDIR + "config" + SLASH;

#define CSV_DELIMITER_s ","
#define CSV_DELIMITER_c ','

#define FIELD_DELIMITER_s ":"
#define FIELD_DELIMITER_c ':'

inline void UNKNOWN_OPTION_MSG(strview opt){ 
  std::cout << "[!] unknown option '" << opt << "'" << std::endl;
}

static inline str last_gpki_error = "no error";
inline void seterror(str err){last_gpki_error = err;} 
inline str lasterror(){return last_gpki_error;}

static int prompt = 1;
