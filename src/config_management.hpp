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


// Static class to manage stuff related with
// configuration files such as loading
// or editing them

class Config
{
private:
	Profile *profile;
	// When loaded, config mappings will look like
	// e.g _conf_vpn[client][key] = val
	//     _conf_vpn[server][key] = val
	//	   _conf_vpn[common][key] = val
	// the adecuate ConfigMap will be populated with
	// each config section, with each section being 
	// loaded in a map key
	ConfigMap _conf_gpkih; // gpkih.conf
	ConfigMap _conf_vpn;   // openvpn.conf
	ConfigMap _conf_pki;   // pki.conf
	// Loads sections from file into given buff
	// like explained just above 
	int load_file(str &&file, ConfigMap& buff);
public:
	// UNUSED
	static inline str VPN_SECTION_COMMON = "common";
	static inline str VPN_SECTION_SERVER = "server";
	static inline str VPN_SECTION_CLIENT = "client";
	
	Config(Profile &profile, CONFIG_FILE file_to_load = CONFIG_ALL);

	ConfigMap *get(CONFIG_FILE sections);
	bool exists(strview key, CONFIG_FILE sections);

	// Dumps vpn configuration (key-map values) to outpath
	// it does file checks and dumps appropiate configuration
	// based on given ENTITY_TYPE (only client|server are valid)
	bool dump_vpn_conf(strview outpath, ENTITY_TYPE type);
	// Will this be useful at all??
	// bool dump_pki_conf(strview outpath);
	// bool dump_gpkih_conf(strview outpath);
	bool dump(strview outpath, CONFIG_FILE files);
	// Sets the contents on config files to the ones 
	// in the ConfigMap mapped values, effectively updating
	// the configuration file in case the map got edited
	// after being loaded
	bool sync(CONFIG_FILE files);
};