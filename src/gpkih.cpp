#include "gpkih.hpp"

#include "consts.hpp"
#include "libs/utils/utils.hpp"
#include "config/Config.hpp"
#include "parse/parser.hpp"
#include "entities/entities.hpp"
#include "signals/signals.cpp"
#include "libs/printing/printing.hpp"

#include "db/profiles.hpp"
#include "db/entities.hpp"

#include <future>

using namespace gpkih;

constexpr size_t gpkihReservedMemory = gurgui::utils::units::toBytes(4,'m');

static constexpr const char* DB_DIRNAME  = "db";
static constexpr const char* CFG_DIRNAME = "config";
static constexpr const char* LOG_DIRNAME = "logs";
static constexpr const char* gpkihConfigFilename = "gpkih.conf";

std::string GPKIH_DIR_ROOT = "";      // proper value set by __setPlatformDependantVariables()
std::string GPKIH_DIR_CONFIG = "";    // proper value set by __setVariables()
std::string GPKIH_DIR_DB = "";        // proper value set by __setVariables()

static std::string GPKIH_FILE_CONFIG;    // initialized by __setVariables()s
static std::string LOG_DIRPATH;

gurgui::logging::Logger *gpkihLogger = nullptr;
GpkihBuffer *gpkihBuffer = nullptr;

bool DRY_RUN = false;
bool SHOW_HEADER = true;

static int __check_gpkih_install_dir(std::string &path) {
    DEBUGF(1, "__check_gpkih_install_dir({})", path);

    if (!fs::exists(path))
    {
        PERROR("Gpkih base dir {} doesn't exist, please create it\n", GPKIH_DIR_ROOT);
        return GPKIH_FATAL;
    }

    return GPKIH_OK;
}

static int __setPlatformDependantVariables()
{
    DEBUG(1, "__setPlatformDependantVariables()");
    
    #ifdef _WIN32
    GPKIH_DIR_ROOT = gurgui::utils::env::get_environment_variable("LOCALAPPDATA");
    #else
    GPKIH_DIR_ROOT = gurgui::utils::env::get_environment_variable("HOME");
    #endif

    if(GPKIH_DIR_ROOT.empty()){
        return GPKIH_FAIL;
    }

    #ifdef _WIN32
    GPKIH_DIR_ROOT += "\\gpkih\\";
    #else
    GPKIH_DIR_ROOT += "/.config/gpkih/";
    #endif

    GPKIH_DIR_ROOT.erase(std::remove_if(GPKIH_DIR_ROOT.begin(), GPKIH_DIR_ROOT.end(), [](char &c){return c == '\0';}),GPKIH_DIR_ROOT.end());

    return GPKIH_OK;
}

static int __setVariables() {
    DEBUG(1, "__setVariables()");

    if (__setPlatformDependantVariables() != GPKIH_OK) {
        return GPKIH_FAIL;
    }

    GPKIH_DIR_DB      = fmt::format("{}{}", GPKIH_DIR_ROOT, DB_DIRNAME)  + SLASH;
    GPKIH_DIR_CONFIG    = fmt::format("{}{}", GPKIH_DIR_ROOT, CFG_DIRNAME) + SLASH;
    LOG_DIRPATH     = fmt::format("{}{}", GPKIH_DIR_ROOT, LOG_DIRNAME) + SLASH;
    GPKIH_FILE_CONFIG      = fmt::format("{}{}", GPKIH_DIR_CONFIG, gpkihConfigFilename);

    // file used by db::profiles namespace to store/load profiles
    db::profiles::dbpath = fmt::format("{}profiles.data", GPKIH_DIR_DB);

    // simple id tracking file (increased by 1)
    db::profiles::idfile = fmt::format("{}id.data", GPKIH_DIR_DB);

    EntityManager::setDir(GPKIH_DIR_DB);
    
    return GPKIH_OK;
}

static int __setBuffer(){
    static GpkihBuffer __mainGpkihBuffer{gpkihReservedMemory};
    if(__mainGpkihBuffer.head() == nullptr){
        PERROR("{}\n",__mainGpkihBuffer.getLastError());
        return GPKIH_FAIL;
    }
    gpkihBuffer = &__mainGpkihBuffer;
    return GPKIH_OK;
}

static int __setLogger(){
    if(gpkihLogger != nullptr){
        PERROR("Gpkih logger already set - {}\n", gpkihLogger->getBaseDir());
        return GPKIH_FAIL;
    }

    gurgui::logging::Logger::setBaseDir(std::move(fs::path(GPKIH_DIR_ROOT)/"logs").string());
    
    auto mSizeView = Config::get("logs", "maxSize");
    
    for(int i = 0; i < mSizeView.size() -1; ++i){
        if(mSizeView[i] < 48 || mSizeView[i] > 58){
            PERROR("Wrong logs.maxSize syntax, found invalid character '{}'\n", mSizeView[i]);
            return GPKIH_FAIL;
        }
    }
    
    const char unit = std::tolower(mSizeView[mSizeView.size()-1]);
    size_t logSize;

    if(unit == 'g' || unit == 'm' || unit == 'k' || unit == 'b'){
        int num = std::stoi(mSizeView.data(),nullptr,10);
        logSize = gurgui::utils::units::toBytes(num,unit);
    }else if(unit < 48 && unit > 57){
        PERROR("Wrong unit in configuration logs.maxSize '{}'\n", unit);
        return GPKIH_FAIL;    
    }else{
        int num = std::stoi(mSizeView.data(),nullptr,10);
        logSize = gurgui::utils::units::toBytes(num,'b');
    }

    auto iFieldSt = Config::get("logs", "includedFormatFields");
    auto iFields = LF_NONE;
    std::istringstream ss(iFieldSt.data());
    std::string token{};
    
    static std::unordered_map<std::string, logMsgField> __str_ffield_map
    {
        {"type", LF_TYPE},
        {"time", LF_TIME},
        {"content", LF_CONTENT}
    };

    while(getline(ss,token,':')){
        if(__str_ffield_map.find(token) == __str_ffield_map.end()){
            PWARN("Skipping unknown log format field '{}' from logs.includedFormatFields\n", token);
            continue;
        }
        iFields = iFields | __str_ffield_map[token];
    }

    ss.clear();
    token.clear();

    auto iLevelSt = Config::get("logs", "includedLevels");
    auto iLevels = LL_NONE;
    std::istringstream iss(iLevelSt.data());

    static std::unordered_map<std::string,logLevel> __str_level_map
    {
        {"info", LL_INFO},
        {"warning", LL_WARN},
        {"error", LL_ERROR},
    };

    while(getline(iss, token, ':')){
        if(__str_level_map.find(token) == __str_level_map.end()){
            PWARN("Skipping unknown log level '{}' from logs.includedLevels\n", token);
            continue;
        }
        iLevels = iLevels | __str_level_map[token];
    }
    
    static gurgui::logging::Logger __mainGpkihLogger("gpkih",logSize,iFields,iLevels);
    gpkihLogger = &__mainGpkihLogger;
    return GPKIH_OK;
}

// PROGRAM ENTRY POINT
int main(int argc, const char **args) {
  std::vector <std::string> opts(args+1,args+argc);

  if(parsers::parseGlobals(opts) == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  if(__setBuffer() != GPKIH_OK){
    return GPKIH_FAIL;
  }

  if(__setVariables() != GPKIH_OK){
    return GPKIH_FAIL;
  };

  if (__check_gpkih_install_dir(GPKIH_DIR_ROOT) != GPKIH_OK) {
    return GPKIH_FAIL;
  }

  // Register signal handlers   
  Signals::register_signals();

  // XDDD this has been here forever broski troski TODO - Add PROPER checks for openssl - openvpn existence (static lib with utils to check and execute commands on both win|lin)

  // Launch task to load `gpkih.conf` configuration
  auto loadGpkihConfigTask = std::async(std::launch::async, Config::load, GPKIH_FILE_CONFIG);

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