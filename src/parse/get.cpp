#include "parser.hpp"
#include <unordered_map>

using namespace gpkih;

static std::unordered_map<str, CONFIG_FILE> map_str_config_file
{
	{"gpkih", CONFIG_GPKIH},
	{"pki", CONFIG_PKI},
	{"vpn", CONFIG_VPN}
};

static void print_scope_change(strview scope)
{
	fmt::print(fg(WHITE) | EMPHASIS::bold, "scope: ");
	fmt::print(S_SUCCESS, "{}\n", scope);
}

// get <profile> <config>.<section>.<prop>
// if no profile given - gpkih is assumed
// if @ is used, the context is changed until
// another @ is encountered or all args are 
// parsed, e.g 

// ./gpkih get test @pki key.size key.algorithm | @pki indicates the context (pki.conf) so instead of pki.key.size *.*.algorithm, u omit pki
// ./gpkih get test @vpn.client remote			| @vpn.client sets the current scope to vpn.conf section [client]
// ./gpkih get		@behaviour					| automaticly the context is 'gpkih' so using scope resolutions already treat it as gpkih.

using Formatter = gpkih::experimental::Formatter;
using FormatInfo = gpkih::experimental::FormatInfo;

// input -> <file>.<section>.<key> | output -> std::tuple<str,str,str>{file,section,key}
static std::tuple<str,str,str> __unpack_input(strview user_input)
{
	auto first_dot = std::find(user_input.begin(), user_input.end(), '.');
	str file(user_input.begin(), first_dot);
	if(first_dot == user_input.end()){
		return std::make_tuple(std::move(file),"","");
	}

	auto second_dot = std::find(first_dot + 1, user_input.end(), '.');
	str section(first_dot+1,second_dot);
	if(second_dot == user_input.end()){
		return std::make_tuple(std::move(file), std::move(section), "");
	}

	str key(second_dot+1, user_input.end());
	return std::make_tuple(std::move(file),std::move(section),std::move(key));
}

// input -> <section>.<key> | output --> std::tuple<str,str>{section,key}
static std::tuple<str,str> __unpack_scoped_input(strview user_input)
{
	auto dot = std::find(user_input.begin(), user_input.end(), '.');
	str section(user_input.begin(), dot);
	if(dot == user_input.end()){
		return std::make_tuple(std::move(section), "");
	}
	str key(dot+1,user_input.end());
	return std::make_tuple(std::move(section), std::move(key));
}

// Prints requested sections | properties from config file
// called when using @<pki|vpn|gpkih>
// stops when @ is encountered, indicating next session
// @returns index of next unprocessed opt in vector or -1 on error
static int __handle_scope_printing (const char *scope, ConfigMap& config, std::vector<str> &opts)
{
	if(map_str_config_file.find(scope) == map_str_config_file.end()){
		PWARN("unknown scope '{}'\n", scope);
		return GPKIH_FAIL;
	}

	// ./gpkih get <...> knowing that <...> first arg 
	// its not a known profile, so assume gpkih scope and parse accordingly 
	print_scope_change(scope);

	auto finfo = Formatter::gpkih_formatinfo();
	Formatter formatter(finfo);

	for(int i = 0; i < opts.size(); ++i){
		strview opt = opts[i];
		if(opt[0] == '@'){
			// got next scope, return current pos
			return i;
		}
		auto dot_iter = std::find(opt.begin(), opt.end(), '.');
		str desired_section(opt.begin(), dot_iter);

		if(config.find(desired_section) == config.end()){
			PWARN("unknown section '{}'\n", desired_section);
		}

		// Reference to desired section inside config for the sake of simplicity on further calls
		std::unordered_map<str,str> &section = config[desired_section];

		str key(dot_iter+1, opt.end());
		
		formatter.print_headers(desired_section);
		if(key.empty()){
			// print full section
			for(const auto &kv : section){
				formatter.print_keyval(kv.first, kv.second);
			}
			continue;
		}

		if(section.find(key) == section.end()){
			PWARN("key '{}' doesn't exist in section '{}'\n", key, desired_section);
			continue;
		}

		// print specific key
		formatter.print_keyval(key, section[key]);
	}

	return opts.size();
}

// Prints every section | keyval from `gpkih.conf`
static void __handle_gpkih_full_printing()
{
	print_scope_change("gpkih");

	auto finfo = Formatter::gpkih_formatinfo();

	Formatter formatter(finfo);
	
	ConfigMap& conf = Config::_conf_gpkih;

	for (const auto& section : conf) {
		formatter.print_headers(section.first);
		for (const auto& keyval : section.second) {
			formatter.print_keyval(keyval.first, keyval.second);
		}
	}
}

static inline std::unordered_map<str, ConfigMap*> get_map(ProfileConfig &config)
{
	return {
		{"gpkih",&Config::_conf_gpkih},
		{"pki", config.get(CONFIG_PKI)},
		{"vpn", config.get(CONFIG_VPN)}
	};
};

int parsers::get(std::vector<str> opts)
{
	if (opts.empty()) {
		// print gpkih general configuration
		__handle_gpkih_full_printing();
		return GPKIH_OK;
	}

	// Check profile
	strview profile_name = opts[0];

	Profile profile;
	if (db::profiles::load(profile_name, profile)) {
		// assumed 'profile' does not exist, so parse 
		// following opts assuming `gpkih.conf` is being
		// requested since it's the global configuration
		__handle_scope_printing("gpkih", Config::_conf_gpkih, opts);
		return GPKIH_OK;
	};

	opts.erase(opts.begin());

	// Profile exists, load its config
	ProfileConfig pconfig(profile);
	if (pconfig.succesfully_loaded != GPKIH_OK) {
		return GPKIH_FAIL;
	}
	
	auto scope_config_map = get_map(pconfig);
	// Iterate on args and print requested pconfiguration
	//experimental::Formatter format(experimental::Formatter::FormatInfo{10, C_ALLIGN});

	Formatter formatter(Formatter::gpkih_formatinfo());

	for (int i = 0; i < opts.size(); ++i) {
		strview opt = opts[i];
		if (opt[0] == '@') {
			// Scoped printing
			str scope(opt.begin()+1, std::find(opt.begin(), opt.end(), '.')); // e.g @gpkih.wiski.troski -> gpkih | @vpn.client -> vpn | @pki -> pki
			
			if(scope_config_map.find(scope) != scope_config_map.end()){
				i = __handle_scope_printing(scope.c_str(), *scope_config_map[scope], opts);
			}
			continue;
		}

		const auto [file, section, key] = __unpack_input(opt);
		
		if(scope_config_map.find(file) == scope_config_map.end()){
			PWARN("unknown file scope '{}'\n", file);
			continue; 
		}

		auto &map = *scope_config_map[file];

		// scope exists
		if(section.empty()){
			// print whole file	
			for(const auto &kv : map){
				formatter.print_headers(kv.first);
				for(const auto &conf : kv.second){
					formatter.print_keyval(conf.first,conf.second);
				}
			}
			continue;
		}

		if(map.find(section) == map.end()){
			PWARN("unknown section '{}' in file '{}'\n", section, file);
			continue;
		}

		auto &section_map = map[section];
		// scope and section exist
		if(key.empty()){
			// print whole section
			formatter.print_headers(file+'.'+section);
			for(const auto &kv : section_map){
				formatter.print_keyval(kv.first, kv.second);
			}
			continue;	
		}

		
		if(section_map.find(key) == section_map.end()){
			PWARN("unknown key '{}' in section '{}' from file '{}'\n", key, section, file);
			continue;
		}

		formatter.print_keyval(key, section_map[key]);
		// print specific property from file's section, e.g vpn.client.remote
	}
	return GPKIH_OK;
}