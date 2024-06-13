#include "gpkih.hpp"

#include <future>
#include "memory/memmgmt.hpp"
#include "parse/parser.hpp"
#include "logger/signals.hpp"
#include "utils/utils.hpp"

using namespace gpkih;

constexpr static size_t __dynamic_memory_size = 4096 * 1024; // 4MB

/* Directory names */
static constexpr const char* DB_DIRNAME  = "db";
static constexpr const char* CFG_DIRNAME = "config";
static constexpr const char* LOG_DIRNAME = "logs";

/* Current path */
static std::string CURRENT_PATH;

std::string GPKIH_BASEDIR = "";   // proper value set by __setPlatformDependantVariables()
std::string CONF_DIRPATH = "";    // proper value set by __setVariables()
std::string DB_DIRPATH = "";      // proper value set by __setVariables()

static std::string CONF_GPKIH;    // initialized by __setVariables()s

static std::string gpkihConfigFilename = "gpkih.conf";
static std::string LOG_DIRPATH;

Logger *gpkihLogger = nullptr;
char *serialPath = nullptr;
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

static int __setPlatformDependantVariables()
{
    PDEBUG(2,"__setPlatformDependantVariables()");

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

static int __setVariables() {
    PDEBUG(2,"__setVariables()");

    if (__setPlatformDependantVariables() != GPKIH_OK) {
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

static int __setBuffer(){
    static Buffer __mainGpkihBuffer(__dynamic_memory_size);
    if(__mainGpkihBuffer.head() == nullptr){
        PERROR("{}\n",__mainGpkihBuffer.getLastError());
        return GPKIH_FAIL;
    }
    gpkihBuffer = &__mainGpkihBuffer;
    return GPKIH_OK;
}

static int __setLogger(){
    if(gpkihLogger != nullptr){
        return GPKIH_FAIL;
    }
    Logger::setBaseDir(std::move(fs::path(GPKIH_BASEDIR)/"logs").string());
    static Logger __mainGpkihLogger(fs::path(GPKIH_BASEDIR)/"gpkih");
    gpkihLogger = &__mainGpkihLogger;
    return GPKIH_OK;
}

static int __parseGlobals(std::vector<std::string> &opts){
    for(int i = 0; i < opts.size();){
        std::string_view opt = opts[i];
        if(opt == "-q" || opt == "--quiet"){
            ENABLE_PRINTING = false;
            opts.erase(opts.begin()+i);
        }else if(opt == "-dr" || opt == "--dryrun"){
            DRY_RUN = true;
            opts.erase(opts.begin()+i);
        }else if(opt == "-nh" || opt == "--noheader"){
            SHOW_HEADER = false;
            opts.erase(opts.begin()+i);
        }
        else if(opt == "-debug" || opt == "--debug"){
            #ifndef GPKIH_ENABLE_DEBUGGING
            PWARN("This version of gpkih wasn't compiled with debugging capabilities\n");
            PHINT("For such thing, you can compile gpkih using the setup script: ./setup -d GPKIH_ENABLE_DEBUGGING=ON\n");
            #endif
    
            if(i+1 == opts.size()){
                #ifndef GPKIH_ENABLE_DEBUGGING
                opts.erase(opts.begin()+i);
                continue;
                #endif
                PERROR("Debug level must be given\n");
                return GPKIH_FAIL;
            }
    
            int debugLevel = std::stoi(opts[i+1]);
    
            if(debugLevel > 0 && debugLevel <= 3){
                ENABLE_DEBUG_MESSAGES = true;
                DEBUG_LEVEL = debugLevel;
                opts.erase(opts.begin()+i,opts.begin()+i+2);
                ++i;
                continue;
            }

            #ifdef GPKIH_ENABLE_DEBUGGING
            PERROR("Debug level must be 0-3 (both included)\n");
            return GPKIH_FAIL;
            #endif
            
        }else{
            ++i;    
        }
    }
  return GPKIH_OK;
}

// PROGRAM ENTRY POINT
int main(int argc, const char **args) {
  std::vector <std::string> opts(args+1,args+argc);

  if(__parseGlobals(opts) == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  if(__setBuffer() != GPKIH_OK){
    return GPKIH_FAIL;
  }

  if(__setVariables() != GPKIH_OK){
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
    return GPKIH_FAIL;
  };

  // Wait for Config::load to finish
  if (loadGpkihConfigTask.get() != GPKIH_OK) {
    return GPKIH_FAIL;
  }

  if(__setLogger() == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  // Parse options
  if (parsers::parse(opts) != GPKIH_OK) {
    return GPKIH_FAIL;
  }

  // Everything went well
  return GPKIH_OK;
}