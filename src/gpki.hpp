#pragma once
#include "logger/logger.hpp"

constexpr static inline size_t gpkih_magic_number = 0x5f67706b69685f;

// Custom typenames
using str     = std::string;
using strview = std::string_view;
using sstream = std::stringstream;

using ui8     = uint8_t;
using ui16    = uint16_t;
using ui32    = uint32_t;
using ui64    = uint64_t;

// Custom namespace names
namespace fs = std::filesystem;

constexpr char EOL = '\n';

#ifdef _WIN32

/* WINDOWS STUFF */
#include <Windows.h>
constexpr char SLASH = '\\';
constexpr const char *VPN_CONFIG_EXTENSION = "ovpn";

#else

/* LINUX STUFF */
constexpr char SLASH = '/';
constexpr const char *VPN_CONFIG_EXTENSION = "conf";
#endif

/* External globals - initialized in gpkih.cpp */
extern std::string vpn_conf_filename;
extern std::string pki_conf_filename;

/* External globals - initialized by main() */
extern std::string GPKIH_BASEDIR;
extern std::string CONF_DIRPATH;
extern std::string DB_DIRPATH;

/* External globals - initialized as NULL, value given on appropiate context (action) */
extern char *serial_path;
extern uint64_t serial_path_len;

/* Error */
extern std::string last_gpkih_error;
extern void printlasterror();

template<typename ...Args>
static inline void seterror(std::string fmt, Args&&... args){
  last_gpkih_error = std::move(fmt::format(fmt,std::forward<Args>(args)...));
}

/* Custom return codes */
enum class GPKIH_RETURN_CODES {
  /* ALL GOOD */
  __we_good = 0,
#define GPKIH_OK static_cast<int>(GPKIH_RETURN_CODES::__we_good)
  __we_notgood = 1,
  
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
  __found_entity = 1,

  /* DATABASE RELATED CODES */
#define ENTITY_FOUND static_cast<int>(GPKIH_RETURN_CODES::__found_entity)
  /* */
};