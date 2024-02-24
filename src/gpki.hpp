#pragma once
#include "printing.hpp"
#include <filesystem>

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

inline str EOL = "\n";

#ifdef _WIN32
#include <Windows.h>
/* WINDOWS STUFF */
inline str CURRENT_PATH = fs::current_path().string();
inline char SLASH = '\\';
inline str GPKIH_BASEDIR = str(std::getenv("LOCALAPPDATA")) + "\\gpkih\\";
inline str VPN_CONFIG_EXTENSION = "ovpn";
#else
#include <sys/signal.h>
/* LINUX STUFF */
inline str CURRENT_PATH = fs::current_path();
inline char SLASH = '/';
inline str GPKIH_BASEDIR = str(std::getenv("HOME")) + "/.config/gpkih/";
inline str VPN_CONFIG_EXTENSION = "conf";
#endif


inline str DB_DIRNAME = "db";
inline str DB_DIRPATH = GPKIH_BASEDIR + DB_DIRNAME + SLASH;

inline str CONF_DIRNAME = "config";
inline str CONF_DIRPATH = GPKIH_BASEDIR + CONF_DIRNAME + SLASH;

inline str vpn_conf_filename = "openvpn.conf";
inline str pki_conf_filename = "pki.conf";
inline str gpkih_conf_filename = "gpkih.conf";

inline str CONF_GPKIH = GPKIH_BASEDIR + CONF_DIRNAME + SLASH + gpkih_conf_filename;

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
  __found_entity = 1,
  /* DATABASE RELATED CODES */
#define ENTITY_FOUND static_cast<int>(GPKIH_RETURN_CODES::__found_entity)
  /* */
};

// TODO - split error management to its own file + class
inline str last_gpki_error = "no error";

template<typename ...Args>
inline void seterror(std::string fmt, Args&&... args){
  last_gpki_error = std::move(fmt::format(fmt,std::forward<Args>(args)...));
}
inline str lasterror() { return last_gpki_error; }
inline void printlasterror() {
	if (last_gpki_error == "no error" || last_gpki_error.empty()) {
		fmt::print(fg(LGREEN) | EMPHASIS::bold, "no error");
	}else {
		PERROR(last_gpki_error); 
	}
}