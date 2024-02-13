#pragma once
#include <csignal>
#include <future>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <map>
#include <unordered_map>
#include <fmt/format.h>
#include "printing.hpp"
// Custom typenames
using str = std::string;
using strview = std::string_view;
using sstream = std::stringstream;
using ui8 = uint8_t;
using ui16 = uint16_t;
using ui32 = uint32_t;
using ui64 = uint64_t;

// Custom namespace names
namespace fs = std::filesystem;

#ifdef _WIN32
/* WINDOWS STUFF */
static inline str CURRENT_PATH = fs::current_path().string();
static inline str SLASH = "\\";
static inline str BASEDIR = str(std::getenv("LOCALAPPDATA")) + "\\gpkih\\";
static inline str EOL = "\r\n";
static inline str VPN_CONFIG_EXTENSION = "ovpn";
#else
/* LINUX STUFF */
static inline str CURRENT_PATH = fs::current_path();
static inline str SLASH = "/";
static inline str BASEDIR = str(std::getenv("HOME")) + "/.config/gpkih/";
static inline str EOL = "\n";
static inline str VPN_CONFIG_EXTENSION = "conf";
#endif

static inline str DB_DIRNAME = "db";
static inline str CONF_DIRNAME = "config";

static inline str DB_DIRPATH = BASEDIR + DB_DIRNAME + SLASH;
static inline str CONF_DIRPATH = BASEDIR + CONF_DIRNAME + SLASH;

static inline str gpkih_conf_filename = "gpkih.conf";

#define CSV_DELIMITER_s ","
#define CSV_DELIMITER_c ','

static inline str last_gpki_error = "no error";
template<typename ...Args>
static inline void seterror(std::string fmt, Args&&... args){
  last_gpki_error = fmt::format(fmt,std::forward<Args>(args)...);
}
static inline str lasterror() { return fmt::format(S_ERROR,last_gpki_error); }
