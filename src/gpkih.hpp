#pragma once
#include "libs/logger/logger.hpp" // Logger gpkihLogger
#include "libs/memory/buffer.hpp" // Buffer gpkihBuffer
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

constexpr const char *GPKIH_VERSION = "1.0";
constexpr const char *GPKIH_DESCRIPTION_BRIEF = "CLI tool to manage self-signed PKI";

constexpr const char *vpnConfFilename = "openvpn.conf";
constexpr const char *pkiConfFilename = "pki.conf";
constexpr const char *opensslConfFilename = "gopenssl.conf";

extern bool DRY_RUN;
extern bool SHOW_HEADER;

extern std::string GPKIH_BASEDIR;
extern std::string CONF_DIRPATH;
extern std::string DB_DIRPATH;

// Buffer instance to manage dynamically allocated memory, used by any part of the program
// that would require allocating dynamic memory (malloc()) started by main() to 'MAYBE, NOT YET'
// allow modification of the buffer size from configuration file 'gpkih.conf'
extern gurgui::memory::Buffer *gpkihBuffer;

// TODO - Give each profile their own logger instance and don't only log info (warnings and errors)
extern gurgui::logging::Logger *gpkihLogger;

/* Custom return codes */
enum class GPKIH_RETURN_CODES : int{
  __we_good = 0,
  __we_bad = 1,  
};
constexpr int GPKIH_OK = static_cast<int>(GPKIH_RETURN_CODES::__we_good);
constexpr int GPKIH_FAIL = static_cast<int>(GPKIH_RETURN_CODES::__we_bad);

#define ADD_LOG gpkihLogger->addLog
#define BUFFER_PTR gpkihBuffer;
#define BUFFER *gpkihBuffer
#define ALLOCATE gpkihBuffer->allocate
#define AVAILABLE_MEMORY gpkihBuffer->available()
#define CALLOCATE gpkihBuffer->allocate_and_copy
#define FREE_MEMORY_BLOCK gpkihBuffer->freeblock
#define BUFFER_DUMP gpkihBuffer->dump
#define ENOUGH_AVAILABLE(x) static_cast<bool>(gpkihBuffer->available()-x > 0)