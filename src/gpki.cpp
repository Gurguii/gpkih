#include "config/config_management.hpp"
/* Parser & subparsers */
#include "parse/parser.hpp"
/* Database (csv) manipulation */
#include "db/database.hpp"
/* Available actions */
#include "actions/actions.hpp"
/* Help functions */
#include "help/help.hpp"

/* [testing] class 'Error' to add/retrieve error related stuff */
#include "logger/error_management.hpp"
/* [testing] class 'Logger' for logging */
#include "logger/logger.hpp"
/* [testing] class 'Signals' for signal related stuff */
#include "logger/signals.hpp"

#include <future> // std::async() std::future<>()


static int check_gpkih_install_dir() {
  if (!fs::exists(GPKIH_BASEDIR)) {
      PINFO("Creating gpkih source dir - '{}'\n", GPKIH_BASEDIR);
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
  }
  return GPKIH_OK;
}

// PROGRAM ENTRY POINT
int main(int argc, const char **args) {
  // Print starting msg
  PROGRAMSTARTING();

  // Register signal handlers
  gpkih::Signals::register_signals();

  // Check if gpkih base dir is created, else try to create it
  if (check_gpkih_install_dir() != GPKIH_OK) {
    printlasterror();
    return -1;
  }

  // Create logger instance which will:
  // 1. call Logger::start() upon construction
  // 2. call Logger::wait() upon destruction
  auto logger = gpkih::Logger::get();

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
