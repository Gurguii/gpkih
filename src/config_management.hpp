#pragma once
#include "gpki.hpp"
#include "parse/parser.hpp"
#include "structs.hpp"
#include <unordered_map>
#include <future>

static inline str vpn_conf_filename = "openvpn.conf";
static inline str pki_conf_filename = "pki.conf";
static inline str gpkih_conf_filename = "gpkih.conf";

using ConfigMap = std::unordered_map<str,std::unordered_map<str,str>>;

enum class CONFIG_FILE
{
	_all = 15,
	#define CONFIG_ALL CONFIG_FILE::_all
	_vpn = 2,
	#define CONFIG_VPN CONFIG_FILE::_vpn
	_pki = 4,
	#define CONFIG_PKI CONFIG_FILE::_pki
	_gpkih = 8,
	#define CONFIG_GPKIH CONFIG_FILE::_gpkih
};

static inline std::vector<CONFIG_FILE> _CONFIG_FILE_files{CONFIG_VPN,CONFIG_PKI};
static inline CONFIG_FILE operator|(CONFIG_FILE lo, CONFIG_FILE ro){
	return static_cast<CONFIG_FILE>(static_cast<uint8_t>(lo) | static_cast<uint8_t>(ro));
}
static inline bool operator&(CONFIG_FILE lo, CONFIG_FILE ro){
	return static_cast<bool>(static_cast<uint8_t>(lo) & static_cast<uint8_t>(ro));
}

#define GPKI_OK 0
enum class FILE_ERROR {
  __doesnt_exist = 2,
#define F_NOEXIST static_cast<int>(FILE_ERROR::__doesnt_exist)
  __no_read = 4,
#define F_NOREAD static_cast<int>(FILE_ERROR::__no_read)
  __no_write = 8,
#define F_NOWRITE static_cast<int>(FILE_ERROR::__no_write)
  __cant_open = 16,
#define F_NOOPEN static_cast<int>(FILE_ERROR::__cant_open)
};

// Static class to manage stuff related with
// configuration files such as loading
// or editing them

class Config
{
private:
	// When loaded, config mappings will look like
	// e.g _conf_vpn[client][key] = val
	//     _conf_vpn[server][key] = val
	//	   _conf_vpn[common][key] = val
	// the adecuate ConfigMap will be populated with
	// each config section, with each section being 
	// loaded in a map key
	static inline ConfigMap _conf_gpkih; // gpkih.conf
	static inline ConfigMap _conf_vpn; // openvpn.conf
	static inline ConfigMap _conf_pki; // pki.conf
	// Loads sections from file into given buff
	// like explained just above 
	static int load_file(str &&file, ConfigMap& buff);
public:
	// Must be called before any further action
	static int load(Profile &profile, CONFIG_FILE file_to_load = CONFIG_ALL);

	static ConfigMap *get(CONFIG_FILE sections);
	static bool exists(strview &key, CONFIG_FILE sections);
};