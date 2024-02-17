
#pragma once
#include <csignal>
#include <future>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <map>
#include <unordered_map>
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
CURRENT_PATH = fs::current_path().string();
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

static inline str vpn_conf_filename = "openvpn.conf";
static inline str pki_conf_filename = "pki.conf";
static inline str gpkih_conf_filename = "gpkih.conf";

static inline str CONF_GPKIH = BASEDIR + CONF_DIRNAME + SLASH + gpkih_conf_filename;

#define CSV_DELIMITER_s ","
#define CSV_DELIMITER_c ','

enum class GPKIH_RETURN_CODES {
  /* ALL GOOD */
  __we_good = 0,
#define GPKIH_OK static_cast<int>(GPKIH_RETURN_CODES::__we_good)
  __we_notgood = -1,
  /* FAIL - call lasterror() for more info */
#define GPKIH_FAIL static_cast<int>(GPKIH_RETURN_CODES::__we_notgood)
  /* FILE RELATED CODES */
  __doesnt_exist = 2,
#define F_NOEXIST static_cast<int>(GPKIH_RETURN_CODES::__doesnt_exist)
  __no_read = 4,
#define F_NOREAD static_cast<int>(GPKIH_RETURN_CODES::__no_read)
  __no_write = 8,
#define F_NOWRITE static_cast<int>(GPKIH_RETURN_CODES::__no_write)
  __cant_open = 16,
#define F_NOOPEN static_cast<int>(GPKIH_RETURN_CODES::__cant_open)
  __cant_create = 32,
#define F_NOCREATE static_cast<int>(GPKIH_RETURN_CODES::__cant_create)
  
  /* */
};

// TODO - split error management to its own file + static class
static inline str last_gpki_error = "no error";
template<typename ...Args>
static inline void seterror(std::string fmt, Args&&... args){
  last_gpki_error = fmt::format(fmt,std::forward<Args>(args)...);
}
static inline str lasterror() { return fmt::format(S_ERROR,last_gpki_error); }