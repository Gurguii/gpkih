#include "enums.hpp"
#include "../entities/entities.hpp"
#include <filesystem>

namespace gpkih
{

class ProfileConfig
{
private:
  CONFIG_FILE succesfullyLoadedFiles = CFILE_NONE;
  std::filesystem::path vpnConfigPath;
  std::filesystem::path pkiConfigPath;
  Profile &profile;
  // When loaded, config mappings will look like
  // e.g   vpnConfig[client][key] = val
  //       vpnConfig[server][key] = val
  //	     vpnConfig[common][key] = val
  // the adecuate ConfigMap will be populated with
  // each config section, with each section being
  // loaded in a map key
  ConfigMap vpnConfig;
  ConfigMap pkiConfig;
public:
  /// @return lastError which indicates what failed on the last GPKIH_FAIL returned by any member function
  /// @note this is experimental, used by set2()
  const char *lastError();

  /// @return CONFIG_FILE with the succesfully loaded configuration files, specific config file can be checked by doing CONFIG_FILE & (CFILE_VPN | CFILE_PKI)
  const CONFIG_FILE loadedFiles();

  /// @brief Print requested file/s, by default prints both VPN - PKI
  void print(CONFIG_FILE files = CFILE_ALL);

  /// @brief ProfileConfig constructor
  ProfileConfig(Profile &profile, CONFIG_FILE file_to_load = CFILE_ALL);

  /// @brief Builds a Subject with values from pki.subject.*
  /// @return Subject instance with the default values in the configuration
  Subject default_subject();

  /// @brief Dumps common vpn config + client|server specific configuration (depending on ENTITY_TYPE) to outpath.
  /// @return true|false indicating dumping success|failure
  /// @note: this function DOES NOT add any inlined certificate/key to the outpath
  bool dump_vpn_conf(std::filesystem::path &outpath, ENTITY_TYPE type);
  
  /// @brief retrieve const pointer to desired file's mapped config
  /// @return pointer to mapped config or nullptr
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

} // namespace gpkih::config