#include "gpki.hpp"
#include "logger/logger.hpp"
#include "logger/signals.hpp"
#include "parse/parser.hpp"
#include "db/database.hpp"

#include <stdlib.h>
#ifdef _WIN32
#include <Windows.h>
str CURRENT_PATH = fs::current_path().string();
char SLASH = '\\';
str GPKIH_BASEDIR = "";
str VPN_CONFIG_EXTENSION = "ovpn";
#else
str CURRENT_PATH = fs::current_path();
char SLASH = '/';
str GPKIH_BASEDIR = str(std::getenv("HOME")) + "/.config/gpkih/";
str VPN_CONFIG_EXTENSION = "conf";
#endif

/* DATABASE */
str DB_DIRNAME = "db";
str DB_DIRPATH = GPKIH_BASEDIR + DB_DIRNAME + SLASH;

/* CONFIGURATION */
str CONF_DIRNAME = "config";
str CONF_DIRPATH = GPKIH_BASEDIR + CONF_DIRNAME + SLASH;

/* CONFIGURATION FILENAMES */
str vpn_conf_filename = "openvpn.conf";
str pki_conf_filename = "pki.conf";
str gpkih_conf_filename = "gpkih.conf";

str CONF_GPKIH = GPKIH_BASEDIR + CONF_DIRNAME + SLASH + gpkih_conf_filename;

str CSV_DELIMITER_s = ",";
char CSV_DELIMITER_c = ',';

int check_gpkih_install_dir() {
  PINFO("checking basedir '{}'\n", GPKIH_BASEDIR);
  if (!fs::exists(GPKIH_BASEDIR)) {
    PINFO("base dir doesn't exist\n");
    fflush(stdout);
    str ans;
    PROMPT("About to create gpkih source dir , proceed?", "[y/n]");
    getline(std::cin, ans);
    for (char& c : ans) {c = tolower(c);}
        if(ans != "y" && ans != "yes"){
            PINFO("User refused, exiting ...\n");
            return GPKIH_FAIL;
        }

        PINFO("Creating gpkih source dir '{}'\n", GPKIH_BASEDIR);
        if (!fs::create_directory(GPKIH_BASEDIR)) {
          seterror("Couldn't create directory '{}'\n", GPKIH_BASEDIR);
          return F_NOEXIST;
        };

        // This is the reason the program requires to be executed from the same dir than config/
        // if the gpkih root dir:
        //      [WINDOWS] $env:localappadata\gpkih 
        //      [LINUX] ~/.config/gpkih 
        // doesn't exist
        str configdir = CURRENT_PATH + SLASH + "config";

        // Copy config directory
        fs::copy(configdir, CONF_DIRPATH, fs::copy_options::recursive);
        if (!fs::exists(CONF_DIRPATH) || !fs::is_directory(CONF_DIRPATH)) {
          seterror("couldn't copy '{}' to '{}'\n", configdir, CONF_DIRPATH);
          return F_NOCREATE;
        }

        // Create gpkih db dir
        if (!fs::create_directory(DB_DIRPATH)) {
          seterror("couldn't create gpkih source db dir '{}'\n", DB_DIRPATH);
          return F_NOCREATE;
        };

        // Create gpkih log dir
        if (!fs::create_directory(gpkih::Logger::log_dirpath)) {
          seterror("couldn't create log dir '{}'\n", gpkih::Logger::log_dirpath);
          return F_NOCREATE;
        }
  }else{
    PINFO("basedir '{}' exists\n", GPKIH_BASEDIR);
  }
  return GPKIH_OK;
}

// PROGRAM ENTRY POINT
int main(int argc, const char **args) {
#ifdef _WIN32
    size_t size = 0;
    getenv_s(&size, NULL, 0, "LOCALAPPDATA");
    if (size == 0) {
        PERROR("couldn't get environment variable 'LOCALAPPDATA'\n");
        return GPKIH_FAIL;
    }
    GPKIH_BASEDIR = std::string("\0", size);
    getenv_s(&size, &GPKIH_BASEDIR[0], size, "LOCALAPPDATA");
    GPKIH_BASEDIR += "\\gpkih\\";
    PINFO("GPKIH_BASEDIR set to '{}'", GPKIH_BASEDIR);
#endif

  // Print starting msg
  PROGRAMSTARTING();

  // Check if gpkih base dir is created, else try to create it
  if (check_gpkih_install_dir() != GPKIH_OK) {
    printlasterror();
    return -1;
  }

  // Register signal handlers
  gpkih::Signals::register_signals();

  // Create logger instance which will:
  // 1. call Logger::start() upon construction
  // 2. call Logger::wait() upon destruction
  gpkih::Logger::get();

  // TODO - Add PROPER checks for openssl - openvpn existence

  // Launch task to load `gpkih.conf` configuration
  auto load_gpkih_config = std::async(std::launch::async, gpkih::Config::load);
  
  // Map profiles' csv to db::profiles::existing_profiles{}
  int profile_count = gpkih::db::profiles::initialize();
  if (profile_count < 0) {
    printlasterror();
    return -1;
  };

  // wait for load_gpkih_config to finish
  if (load_gpkih_config.get() != GPKIH_OK) {
    printlasterror();
    return -1;
  }
  
  PINFO("Loaded [{}] profiles\n", profile_count, DB_DIRPATH, SLASH);

  // Parse options
  if (gpkih::parsers::parse(argc - 1, args + 1) != GPKIH_OK) {
    printlasterror();
    return GPKIH_FAIL;
  }

  // Everything went well
  return GPKIH_OK;
}
