#pragma once
#include "../gpki.hpp"
#include "../structs.hpp" // struct Profile | struct Entity | struct Subject
#include "../memmgmt.hpp"

namespace gpkih {
using ConfigMap = std::unordered_map<str, std::unordered_map<str, str>>;


enum class CONFIG_FILE
{
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
  extern ConfigMap _conf_gpkih;

  extern int load(std::string_view filepath);

  extern std::string_view get(std::string_view section, std::string_view key);
  extern void set(std::string_view section, std::string_view key, std::string_view val);

  // to avoid std::find() duplications
  extern bool section_exists(const char * section);
  extern const std::unordered_map<str,str>* const key_exists(const char * key);
  extern bool key_exists(const char * section, const char * key);
}; // namespace Config


// @brief Class to manage profile specific configuration - pki.conf vpn.conf
class ProfileConfig
{
private:
  // When loaded, config mappings will look like
  // e.g   _conf_vpn[client][key] = val
  //       _conf_vpn[server][key] = val
  //	     _conf_vpn[common][key] = val
  // the adecuate ConfigMap will be populated with
  // each config section, with each section being
  // loaded in a map key
  ConfigMap _conf_vpn{
      {"common", {}}, // common vpn config will be mapped here as key-value strings
      {"client", {}}, // * * client
      {"server", {}}, // * * server
  };
  ConfigMap _conf_pki{
      {"key", {}},    // pki key config will be mapped here
      {"crt", {}},    // * * certificate
      {"csr", {}},    // * * certificate requests
      {"crl", {}},    // * * certificate revocation list
      {"subject", {}},// * * subject defaults (country, state, location, organisation, email)
      {"output",{}},  // certain behaviour when building certificates (create_inline, create_pfx)  
  }
;    
public:
  // Print requested file/s, by default prints both VPN - PKI
  void print(CONFIG_FILE files = CONFIG_ALL);

  // Set to true by ProfileConfig() constructor if files are sucesfully loaded
  bool succesfully_loaded = false;

  // Constructor
  ProfileConfig(Profile &profile, CONFIG_FILE file_to_load = CONFIG_ALL);
  Subject default_subject();

  // @brief Dumps common vpn config + client|server specific configuration (depending on ENTITY_TYPE) to outpath.
  // @return true|false indicating dumping success|failure
  // @note: this function DOES NOT add any inlined certificate/key to the outpath
  bool dump_vpn_conf(fs::path &outpath, ENTITY_TYPE type);
  
  bool dump(std::string_view outpath, CONFIG_FILE files);

  ConfigMap* const get(CONFIG_FILE file);
  ConfigMap& _get(CONFIG_FILE file);

  void set(CONFIG_FILE file, std::string_view section, std::string_view key, std::string_view val);
  
  bool exists(std::string_view key, CONFIG_FILE files);

  // @return true:success false:failure
  // @brief Synchronizes files' contents to values in ConfigMap, effectively updating the values
  bool sync(CONFIG_FILE files);
}; // class ProfileConfig

static inline CONFIG_FILE operator|(CONFIG_FILE lo, CONFIG_FILE ro){
  return static_cast<CONFIG_FILE>(static_cast<uint8_t>(lo) |
                                  static_cast<uint8_t>(ro));
}

static inline bool operator&(CONFIG_FILE lo, CONFIG_FILE ro){
  return static_cast<bool>(static_cast<uint8_t>(lo) & static_cast<uint8_t>(ro));
}

} // namespace gpkih