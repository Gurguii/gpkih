#include "gpki.hpp"
#include "logger/signals.hpp"
#include "parse/parser.hpp"
#include "utils/utils.hpp"


#include "printing/printing.cpp"
#include <cstdlib>
#include <filesystem>
#include <iostream>

using namespace gpkih;

/* Directory names */
static constexpr const char* DB_DIRNAME  = "db";
static constexpr const char* CFG_DIRNAME = "config";
static constexpr const char* LOG_DIRNAME = "logs";

/* Configuration names */
str vpn_conf_filename = "openvpn.conf";
str pki_conf_filename = "pki.conf";

/* Current path */
static str CURRENT_PATH;

static str GPKIH_BASEDIR; // initialized by __set_globals()
str CONF_DIRPATH;         // proper value set by __set_globals()
static str DB_DIRPATH;    // initialized by __set_globals()

static str DB_PROFILES_CSV;

static str CONF_GPKIH;    // initialized by __set_globals()

static str gpkih_conf_filename = "gpkih.conf";
static str LOG_DIRPATH;

static str CSV_DELIMITER_s = ",";
static char CSV_DELIMITER_c = ',';

static int check_gpkih_install_dir(str &path) {
    if (!fs::exists(path))
    {
        str ans;
        PROMPT("About to create gpkih source dir , proceed?", "[y/n]");
        getline(std::cin, ans);
        for (char& c : ans) { c = tolower(c); }
        if (ans != "y" && ans != "yes") {
            PINFO("User refused, exiting ...\n");
            return GPKIH_FAIL;
        }

        PINFO("Creating gpkih source dir '{}'\n", path);
        if (!fs::create_directory(path)) {
            seterror("Couldn't create directory '{}'\n", path);
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
        
        // Create db - logs dir
        for (const std::string& path : { DB_DIRPATH, LOG_DIRPATH }) {
            if (!fs::create_directory(path)) {
                seterror("couldn't create dir '{}'\n", path);
                return F_NOCREATE;
            }
        };
    }
  return GPKIH_OK;
}

static int __set_platform_dependant_variables()
{
    #ifdef _WIN32
    GPKIH_BASEDIR = utils::env::get_environment_variable("LOCALAPPDATA");
    #else
    GPKIH_BASEDIR = utils::env::get_environment_variable("HOME");
    #endif

    if(GPKIH_BASEDIR.empty()){
        return GPKIH_FAIL;
    }

    #ifdef _WIN32
    GPKIH_BASEDIR += "\\gpkih\\";
    #else
    GPKIH_BASEDIR += "/.config/gpkih/";
    #endif

    GPKIH_BASEDIR.erase(std::remove_if(GPKIH_BASEDIR.begin(), GPKIH_BASEDIR.end(), [](char &c){return c == '\0';}),GPKIH_BASEDIR.end());

    return GPKIH_OK;
}

static int set_variables() {

    if (__set_platform_dependant_variables() != GPKIH_OK) {
        return GPKIH_FAIL;
    }

    CURRENT_PATH    = fs::current_path().string();
    DB_DIRPATH      = fmt::format("{}{}{}", GPKIH_BASEDIR, DB_DIRNAME, SLASH);
    CONF_DIRPATH    = fmt::format("{}{}{}", GPKIH_BASEDIR, CFG_DIRNAME, SLASH);
    LOG_DIRPATH     = fmt::format("{}{}{}", GPKIH_BASEDIR, LOG_DIRNAME, SLASH);
    CONF_GPKIH      = fmt::format("{}{}", CONF_DIRPATH, gpkih_conf_filename);
    DB_PROFILES_CSV = fmt::format("{}{}", CONF_DIRPATH, "profiles.csv");

    // db::profiles::dbpath
    db::profiles::dbpath = fmt::format("{}profiles.csv", DB_DIRPATH);
    
    return GPKIH_OK;
}

// PROGRAM ENTRY POINT
int main(int argc, const char **args) {

  for(int i = 0; i < argc; ++i){
    if(!strcmp(args[i], "--debug")){
        ENABLE_DEBUG_MESSAGES = true;
        args[i] = NULL;
        --argc;
    }
  }

  PDEBUG("setting global variables");
  if(set_variables() != GPKIH_OK){
    printlasterror();
    return GPKIH_FAIL;
  };

  
  
  PDEBUG("checking base dir");
  if (check_gpkih_install_dir(GPKIH_BASEDIR) != GPKIH_OK) {
    printlasterror();
    return GPKIH_FAIL;
  }


  // Register signal handlers   
  Signals::register_signals();

  // TODO - Add PROPER checks for openssl - openvpn existence (static lib with utils to check and execute commands on both win|lin)

  // Launch task to load `gpkih.conf` configuration
  auto load_gpkih_config = std::async(std::launch::async, Config::load, CONF_GPKIH);

  // Map profiles' csv to db::profiles::existing_profiles{}
  size_t profile_count = db::profiles::initialize(DB_DIRPATH + "profiles.csv");
  if (profile_count < 0) {
    printlasterror();
    return -1;
  };

  // wait for Config::load to finish
  if (load_gpkih_config.get() != GPKIH_OK) {
    printlasterror();
    return -1;
  }
  
  // Initialize the program logger
  if (Logger::initialize(std::move((fs::path(GPKIH_BASEDIR)/"logs").string())) == false) {
    printlasterror();
    return GPKIH_FAIL;
  };

  auto logger = Logger::get();

  PDEBUG("Loaded [{}] profiles", profile_count, DB_DIRPATH, SLASH);

  // Parse options
  if (parsers::parse(argc - 1, args + 1) != GPKIH_OK) {
    printlasterror();
    return GPKIH_FAIL;
  }

  // Everything went well
  return GPKIH_OK;
}
