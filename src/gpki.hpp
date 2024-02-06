#pragma once
#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <map>
#include <unordered_map>

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
#else
/* LINUX STUFF */
static inline str CURRENT_PATH = fs::current_path();
static inline str SLASH = "/";
static inline str BASEDIR = str(std::getenv("HOME")) + "/.config/gpkih/";
#endif

static inline str DBDIR = BASEDIR + "db" + SLASH;
static inline str CONFDIR = BASEDIR + "config" + SLASH;

static inline str template_filename = "templates.conf";
static inline str gpkih_filename = "gpkih.conf";

#define CSV_DELIMITER_s ","
#define CSV_DELIMITER_c ','

static inline str last_gpki_error = "no error";
static inline void seterror(str err) { last_gpki_error = err; }
static inline str lasterror() { return last_gpki_error; }

static int prompt = 1;