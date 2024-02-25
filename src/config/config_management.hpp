#pragma once
#include "../structs.hpp"


namespace gpkih {
using ConfigMap = std::unordered_map<str, std::unordered_map<str, str>>;

enum class CONFIG_FILE {
  _all = 7,
#define CONFIG_ALL CONFIG_FILE::_all
  _vpn = 2,
#define CONFIG_VPN CONFIG_FILE::_vpn
  _pki = 4,
#define CONFIG_PKI CONFIG_FILE::_pki
};

static inline CONFIG_FILE operator|(CONFIG_FILE lo, CONFIG_FILE ro) {
  return static_cast<CONFIG_FILE>(static_cast<uint8_t>(lo) |
                                  static_cast<uint8_t>(ro));
}
static inline bool operator&(CONFIG_FILE lo, CONFIG_FILE ro) {
  return static_cast<bool>(static_cast<uint8_t>(lo) & static_cast<uint8_t>(ro));
}

static inline str skip_chars = "#\n ";
static inline char section_delim_open = '[';
static inline char section_delim_close = ']';
static inline str empty_chars =
    fmt::format("{}{} ", section_delim_open, section_delim_close);

namespace config::gpkih::sections
{
  static inline str metadata = "metadata";
  static inline str behaviour = "behaviour";
}
namespace config::pki::sections
{
  static inline str key = "key";
  static inline str crl = "crl";
  static inline str crt = "crt";
  static inline str subject = "subject";
}
namespace config::openvpn::sections
{
  static inline str common = "common";
  static inline str client = "client";
  static inline str server = "server";
}

struct pki_building_params
{
  str key_size;
};

// Static class to manage gpkih.conf
class Config {
private:
  static inline ConfigMap _conf_gpkih{
      {"metadata", {}}, 
      {"behaviour", {}},
      {"logs", {} },
  };

protected:
  static int load_file(strview path, ConfigMap &buff);

public:
  static inline int load() { return load_file(CONF_GPKIH, _conf_gpkih); }
  static inline strview get(strview section, strview key){return _conf_gpkih[section.data()][key.data()]; }
  static inline void set(strview section, strview key, strview val){
    _conf_gpkih[section.data()][key.data()] =  val.data();
  }
  static inline void print() {
    for (auto &kv : _conf_gpkih) {
      fmt::print("== {} ==\n",kv.first);
      for (auto &kvv : kv.second) {
        fmt::print("{} {}\n",kvv.first, kvv.second);
      }
    }
  }
}; // class Config



// Class to manage profile specific configuration - pki.conf vpn.conf
class ProfileConfig : public Config {
private:
  // When loaded, config mappings will look like
  // e.g _conf_vpn[client][key] = val
  //     _conf_vpn[server][key] = val
  //	   _conf_vpn[common][key] = val
  // the adecuate ConfigMap will be populated with
  // each config section, with each section being
  // loaded in a map key
  ConfigMap _conf_vpn{
      {"common",
       {}}, // common vpn config will be mapped here as key-value strings
      {"client", {}}, // * * client
      {"server", {}}, // * * server
  };
  ConfigMap _conf_pki{
      {"key", {}},    // pki key config will be mapped here
      {"crt", {}},    // * * certificate
      {"csr", {}},    // * * certificate requests
      {"crl", {}},    // * * certificate revocation list
      {"subject", {}} // * * subject defaults (country, state, location,
                      // organisation, common name, email)
  };
    
public:
  inline void print() {
    for (auto &haha : {_conf_vpn, _conf_pki}) {
      for (auto &kv : haha) {
        fmt::print("== {} ==\n",kv.first);
        for (auto &kvv : kv.second) {
          fmt::print("{} {}\n", kvv.first, kvv.second);
        }
      }
    }
  } // class ProfileConfig 


  // Set to true by ProfileConfig() constructor if files are sucesfully loaded
  bool succesfully_loaded = false;
  // Constructor
  ProfileConfig(Profile &profile, CONFIG_FILE file_to_load = CONFIG_ALL);
  Subject default_subject();

  bool dump_vpn_conf(strview outpath, ENTITY_TYPE type);
  bool dump(strview outpath, CONFIG_FILE files);

  ConfigMap* const get(CONFIG_FILE sections);
  ConfigMap& _get(CONFIG_FILE file);
  void set(CONFIG_FILE file, strview section, strview key, strview val);
  
  bool exists(strview key, CONFIG_FILE sections);

  // Sets the contents on config files to the ones
  // in the ConfigMap mapped values, effectively updating
  // the configuration file in case the map got edited
  // after being loaded
  bool sync(CONFIG_FILE files);
};
} // namespace gpkih