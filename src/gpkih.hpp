#pragma once
#include "libs/logger/logger.hpp" // Logger gpkihLogger
#include "buffer.hpp"
#include <filesystem>

namespace fs = std::filesystem;

constexpr char EOL = '\n';

#ifdef _WIN32
#include <Windows.h>
constexpr char SLASH = '\\';
#else
constexpr char SLASH = '/';
#endif

constexpr const char *vpnConfFilename = "openvpn.conf";
constexpr const char *pkiConfFilename = "pki.conf";

extern bool DRY_RUN;
extern bool SHOW_HEADER;

extern std::string GPKIH_BASEDIR;
extern std::string CONF_DIRPATH;
extern std::string DB_DIRPATH;

// Buffer instance to manage dynamically allocated memory, used by any part of the program
// that would require allocating dynamic memory (malloc()) started by main() to 'MAYBE, NOT YET'
// allow modification of the buffer size from configuration file 'gpkih.conf'
extern GpkihBuffer *gpkihBuffer;

// TODO - Give each profile their own logger instance and don't only log info (warnings and errors)
extern gurgui::logging::Logger *gpkihLogger;

/* Custom return codes */
enum class GPKIH_RETURN_CODES : int {
  __we_good = 0,
  __we_bad  = 1,
  __we_done = 2,
};

constexpr int GPKIH_OK = static_cast<int>(GPKIH_RETURN_CODES::__we_good);
constexpr int GPKIH_FAIL = static_cast<int>(GPKIH_RETURN_CODES::__we_bad);
constexpr int GPKIH_FATAL = static_cast<int>(GPKIH_RETURN_CODES::__we_done);
#include "./macros.hpp"