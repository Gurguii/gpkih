#include "gpkih.hpp"

#include <future>
#include "db/mnck.hpp"
#include "memory/memmgmt.hpp"
#include "parse/parser.hpp"
#include "logger/signals.hpp"

using namespace gpkih;

constexpr static size_t __dynamic_memory_size = 1024 * 4096; // 4MB

/* Directory names */
static constexpr const char* DB_DIRNAME  = "db";
static constexpr const char* CFG_DIRNAME = "config";
static constexpr const char* LOG_DIRNAME = "logs";

/* Current path */
static std::string CURRENT_PATH;

std::string GPKIH_BASEDIR = "";   // proper value set by __set_platform_dependant_variables()
std::string CONF_DIRPATH = "";    // proper value set by __set_variables()
std::string DB_DIRPATH = "";      // proper value set by __set_variables()

static std::string CONF_GPKIH;    // initialized by __set_variables()s

static std::string gpkihConfigFilename = "gpkih.conf";
static std::string LOG_DIRPATH;

char *serialPath = NULL;
size_t serialPathLen = 0;

bool DRY_RUN = false;
bool SHOW_HEADER = true;

static int __check_gpkih_install_dir(std::string &path) {
    PDEBUG(2, "__check_gpkih_install_dir()");

    if (!fs::exists(path))
    {
        if(Config::get("behaviour","autoasnwer") == "no"){
            auto ans = PROMPT("About to create gpkih source dir , proceed?", "[y/n]", true);
            if (ans != "y" && ans != "yes") {
                PINFO("User refused, exiting ...\n");
                return GPKIH_FAIL;
            }
        }

        if (!fs::create_directories(path)) {
            PERROR("Couldn't create gpkih's root directory '{}'\n", path);
            return GPKIH_FAIL;
        };

        // This is the reason the program requires to be executed from the same dir than config
        // if the gpkih root dir:
        //      [WINDOWS] $env:localappadata\gpkih 
        //      [LINUX] ~/.config/gpkih 
        // doesn't exist
        std::string configdir = CURRENT_PATH + SLASH + "config";

        // Check if configdir exists, else we are not in the proper location, exit.
        if (fs::exists(configdir) == false) {
            PERROR("couldn't find config dir in current directory '{}', exiting...\n");
            return GPKIH_FAIL;
        }

        // Copy config directory
        fs::copy(configdir, CONF_DIRPATH, fs::copy_options::recursive);
        if (!fs::exists(CONF_DIRPATH) || !fs::is_directory(CONF_DIRPATH)) {
            PERROR("couldn't copy '{}' to '{}'\n", configdir, CONF_DIRPATH);
            return GPKIH_FAIL;
        }
        
        // Create db - logs dir
        for (const std::string& path : { DB_DIRPATH, LOG_DIRPATH }) {
            if (!fs::create_directory(path)) {
                PERROR("couldn't create dir '{}'\n", path);
                return GPKIH_FAIL;
            }
        };
    }
  return GPKIH_OK;
}

static int __set_platform_dependant_variables()
{
    PDEBUG(2,"__set_platform_dependant_variables()");

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

static int __set_variables() {
    PDEBUG(2,"__set_variables()");

    if (__set_platform_dependant_variables() != GPKIH_OK) {
        return GPKIH_FAIL;
    }

    CURRENT_PATH    = fs::current_path().string();
    DB_DIRPATH      = fmt::format("{}{}{}", GPKIH_BASEDIR, DB_DIRNAME, SLASH);
    CONF_DIRPATH    = fmt::format("{}{}{}", GPKIH_BASEDIR, CFG_DIRNAME, SLASH);
    LOG_DIRPATH     = fmt::format("{}{}{}", GPKIH_BASEDIR, LOG_DIRNAME, SLASH);
    CONF_GPKIH      = fmt::format("{}{}", CONF_DIRPATH, gpkihConfigFilename);

    // file used by db::profiles namespace to store/load profiles
    db::profiles::dbpath = fmt::format("{}profiles.data", DB_DIRPATH);

    // simple id tracking file (increased by 1)
    db::profiles::idfile = fmt::format("{}id.data", DB_DIRPATH);

    EntityManager::dbdir = DB_DIRPATH;
    
    return GPKIH_OK;
}

// PROGRAM ENTRY POINT
int main(int argc, const char **args) {
  PDEBUG(1,"main()");
  std::vector <std::string> opts(args+1,args+argc);

  for(int i = 0; i < opts.size();){
    std::string_view opt = opts[i];
    if(opt == "-debug" || opt == "--debug"){
        if(i+1 == opts.size()){
            PERROR("Debug level must be given\n");
            return GPKIH_FAIL;
        }
        int debug_level = std::stoi(opts[i+1]);
        if(debug_level > 0 && debug_level <= 3){
            ENABLE_DEBUG_MESSAGES = true;
            DEBUG_LEVEL = debug_level;
        }else{
            PERROR("Debug level must be 0-3 (both included)\n");
            return GPKIH_FAIL;
        }
        opts.erase(opts.begin()+i,opts.begin()+i+2);
    }else if(opt == "-q" || opt == "--quiet"){
        ENABLE_PRINTING = false;
        opts.erase(opts.begin()+i);
    }else if(opt == "-dr" || opt == "--dryrun"){
        DRY_RUN = true;
        opts.erase(opts.begin()+i);
    }else if(opt == "-nh" || opt == "--noheader"){
        SHOW_HEADER = false;
        opts.erase(opts.begin()+i);
    }else{
        ++i;
    }
  }

  if(Buffer::initialize(__dynamic_memory_size, gpkihBuffer) != BUFF_OK){
    PERROR("Couldn't allocate dynamic memory {} bytes\n", __dynamic_memory_size);
    return GPKIH_FAIL;
  }

  if(__set_variables() != GPKIH_OK){
    return GPKIH_FAIL;
  };

  if (__check_gpkih_install_dir(GPKIH_BASEDIR) != GPKIH_OK) {
    return GPKIH_FAIL;
  }

  // Register signal handlers   
  Signals::register_signals();

  // TODO - Add PROPER checks for openssl - openvpn existence (static lib with utils to check and execute commands on both win|lin)

  // Launch task to load `gpkih.conf` configuration
  auto loadGpkihConfigTask = std::async(std::launch::async, Config::load, CONF_GPKIH);

  // Initialize profiles' db
  size_t profile_count = 0;
  if (db::profiles::initialize(profile_count) == GPKIH_FAIL) {
    loadGpkihConfigTask.wait();
    return -1;
  };

  // Wait for Config::load to finish
  if (loadGpkihConfigTask.get() != GPKIH_OK) {
    return -1;
  }

  /* Initialize program logger */
  Logger::set_basedir(std::move(fs::path(GPKIH_BASEDIR)/"logs").string());
  Logger pl = Logger("gpkih");
  gpkih_logger = &pl;

  // Parse options
  if (parsers::parse(opts) != GPKIH_OK) {
    return GPKIH_FAIL;
  }

  // Everything went well
  return GPKIH_OK;
}