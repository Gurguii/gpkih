#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <filesystem>

// Custom typenames
using str     = std::string;
using strview = std::string_view;
using sstream = std::stringstream;

using ui8     = uint8_t;
using ui16    = uint16_t;
using ui32    = uint32_t;
using ui64    = uint64_t;

namespace fs = std::filesystem;

constexpr char EOL = '\n';

#ifdef _WIN32

/* WINDOWS STUFF */
#include <Windows.h>
constexpr char SLASH = '\\';
constexpr const char *vpnConfigExtension = "ovpn";

#else

/* LINUX STUFF */
constexpr char SLASH = '/';
constexpr const char *vpnConfigExtension = "conf";

#endif

/* Globals */

/** filenames **/
constexpr const char *vpnConfFilename = "openvpn.conf";
constexpr const char *pkiConfFilename = "pki.conf";
constexpr const char *opensslConfFilename = "gopenssl.conf";

/** behaviour **/
extern bool DRY_RUN;
extern bool SHOW_HEADER;

/** external **/
extern char *serialPath;
extern size_t serialPathLen;

extern std::string GPKIH_BASEDIR;
extern std::string CONF_DIRPATH;
extern std::string DB_DIRPATH;

/* Custom return codes */
enum class GPKIH_RETURN_CODES : int{
  __we_good = 0,
#define GPKIH_OK static_cast<int>(GPKIH_RETURN_CODES::__we_good)
  __we_bad = 1,  
#define GPKIH_FAIL static_cast<int>(GPKIH_RETURN_CODES::__we_bad)
};