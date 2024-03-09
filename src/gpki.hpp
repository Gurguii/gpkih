#pragma once
#include <filesystem>

#include "printing/printing.hpp"
#include "logger/logger.hpp"
 
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

extern str vpn_conf_filename;
extern str pki_conf_filename;
extern str CONF_DIRPATH;

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
static inline str last_gpki_error = "no error";

template<typename ...Args>
static inline void seterror(std::string fmt, Args&&... args){
  last_gpki_error = std::move(fmt::format(fmt,std::forward<Args>(args)...));
}
static inline str lasterror() { return last_gpki_error; }
static inline void printlasterror() {
	if (last_gpki_error == "no error" || last_gpki_error.empty()) {
		fmt::print(fg(LGREEN) | EMPHASIS::bold, "no error\n");
	}else {
		fmt::print(fg(RED), last_gpki_error); 
	}
}