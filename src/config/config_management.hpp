#pragma once
#include "../gpkih.hpp"
#include "../structs.hpp" // struct Profile | struct Entity | struct Subject

namespace gpkih {
using ConfigMap = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

enum class CONFIG_FILE
{
    _none = 0,
  #define CONFIG_NONE CONFIG_FILE::_none
    _all = 7,
  #define CONFIG_ALL CONFIG_FILE::_all
    _vpn = 2,
  #define CONFIG_VPN CONFIG_FILE::_vpn
    _pki = 4,
  #define CONFIG_PKI CONFIG_FILE::_pki
    _gpkih = 1,
  #define CONFIG_GPKIH CONFIG_FILE::_gpkih
};
  
// Namespace to manage gpkih.conf
namespace Config
{
  extern ConfigMap gpkihConfig;

  /// @brief load configuration from given path
  /// @param filepath configuration file path
  /// @return GPKIH_OK if file was succesfully loaded else GPKIH_FAIL
  /// @note this file will only load 1 filepath, no matter how many times it gets called
  extern int load(std::string_view filepath);

  /// @brief get the value of a section's key
  /// @return std::string_view with the value
  /// @note the returned value could be empty if the key does not exist
  extern std::string_view get(std::string_view section, std::string_view key);

  /// @brief set a new value of a section key
  /// @return GPKIH_OK if value was changed, GPKIH_FAIL if it didn't
  extern int set(std::string_view section, std::string_view key, std::string_view val);

  /// @brief checks if section exists
  /// @return true if section exists else false
  extern bool section_exists(const char * section);

  /// @brief checks if key exists in any section
  /// @return true if key exists else false
  extern const std::unordered_map<str,str>* const key_exists(const char * key);

  /// @brief checks if key exists in a specific section
  /// @return true if key exists else false
  extern bool key_exists(const char * section, const char * key);

  /// @brief writes configuration to file, updating values made with Config::set() since it was loaded
  /// @return GPKIH_OK if succeeds else GPKIH_FAIL
  extern int sync();
}; // namespace Config

namespace Config::behaviour
{
  static bool autoanswer;
  static bool prompt;
}

// @brief Class to manage profile specific configuration - pki.conf vpn.conf
class ProfileConfig
{
private:
  CONFIG_FILE succesfullyLoadedFiles = CONFIG_NONE;
  fs::path vpnConfigPath;
  fs::path pkiConfigPath;
  Profile &profile;
  // When loaded, config mappings will look like
  // e.g   vpnConfig[client][key] = val
  //       vpnConfig[server][key] = val
  //	     vpnConfig[common][key] = val
  // the adecuate ConfigMap will be populated with
  // each config section, with each section being
  // loaded in a map key
  ConfigMap vpnConfig{
    {"common", {
      {"tls-auth",""},
      {"dev",""},
      {"cipher",""},
      {"proto",""}
    }}, // common vpn config will be mapped here as key-value strings
    {"client", {
      {"verb",""},
      {"installdir",""},
      {"remote-cert-tls",""},
      {"remote",""}
    }}, // * * client
    {"server", {
      {"push",""},
      {"ifconfig-pool-persist",""},
      {"dh",""},
      {"verb",""},
      {"crl-verify",""},
      {"keepalive",""},
      {"status",""},
      {"explicit-exit-notify",""},
      {"port",""},
      {"server",""}
    }}, // * * server
  };
  ConfigMap pkiConfig{
    {"key", {
      {"size",""},
      {"algorithm",""}
    }},    // pki key config will be mapped here
    {"crt", {
      {"days",""},
    }},    // * * certificate
    {"subject",{
      {"email",""},
      {"common_name",""},
      {"organisation",""},
      {"location",""},
      {"state",""},
      {"country",""}
    }}, // * * subject defaults (country, state, location, organisation, email)
    {"output",{
      {"create_pfx",""},
      {"create_inline",""}
    }},  // certain behaviour when building certificates (create_inline, create_pfx)  
  };
public:
  /// @return lastError which indicates what failed on the last GPKIH_FAIL returned by any member function
  /// @note this is experimental, used by set2()
  const char *lastError();

  /// @return CONFIG_FILE with the succesfully loaded configuration files, specific config file can be checked by doing CONFIG_FILE & (CONFIG_VPN | CONFIG_PKI)
  const CONFIG_FILE loadedFiles();

  /// @brief Print requested file/s, by default prints both VPN - PKI
  void print(CONFIG_FILE files = CONFIG_ALL);

  /// @brief ProfileConfig constructor
  ProfileConfig(Profile &profile, CONFIG_FILE file_to_load = CONFIG_ALL);
  Subject default_subject();

  /// @brief Dumps common vpn config + client|server specific configuration (depending on ENTITY_TYPE) to outpath.
  /// @return true|false indicating dumping success|failure
  /// @note: this function DOES NOT add any inlined certificate/key to the outpath
  bool dump_vpn_conf(fs::path &outpath, ENTITY_TYPE type);
  
  bool dump(std::string_view outpath, CONFIG_FILE files);

  /// @brief retrieve const pointer to desired file's mapped config
  /// @return pointer to mapped config or NULL
  ConfigMap* const getptr(CONFIG_FILE file);
  
  /// @brief retrieve desired file's mapped config
  /// @return vpn config if requested, else returns pki config
  ConfigMap& get(CONFIG_FILE file);

  /// @brief attempts to set a new key value
  /// @return GPKIH_OK on succesful value update, else false
  /// @note: this function DOES NOT update contents on the configuration file, sync() must be called after making changes
  int set(CONFIG_FILE file, std::string_view section, std::string_view key, std::string_view nval);

  /// @brief attempts to set a new key value
  /// @return GPKIH_OK if value was changed, else GPKIH_FAIL and informs about the encountered error
  int set2(CONFIG_FILE file, std::string_view section, std::string_view key, std::string_view nval);

  /// @brief indicates if section exists in desired config file
  /// @return true if exists, else false
  bool section_exists(std::string_view section, CONFIG_FILE file);

  /// @brief indicates if key exists in desired config file's section
  /// @return true if exists, else false
  bool key_exists(CONFIG_FILE file, std::string_view section, std::string_view key);

  /// @brief indicates if key exists in desired config file
  /// @return true if key exists, else false
  bool key_exists(std::string_view key, CONFIG_FILE file);

  /// @brief Synchronizes files' contents to values in ConfigMap, effectively updating the values
  /// @return true:success false:failure
  int sync(CONFIG_FILE files);
  
}; // class ProfileConfig

static inline CONFIG_FILE operator|(CONFIG_FILE lo, CONFIG_FILE ro){
  return static_cast<CONFIG_FILE>(static_cast<uint8_t>(lo) |
                                  static_cast<uint8_t>(ro));
}

static inline bool operator&(CONFIG_FILE lo, CONFIG_FILE ro){
  return static_cast<bool>(static_cast<uint8_t>(lo) & static_cast<uint8_t>(ro));
}

} // namespace gpkih