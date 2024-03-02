#include "parser.hpp"
#include <fmt/color.h>
#include <unordered_map>
#include "../experimental/formatter.hpp"
using namespace gpkih;

static void print_scope_change(strview scope)
{
	fmt::print(fg(WHITE) | EMPHASIS::bold, "scope: ");
	fmt::print(S_SUCCESS, "{:^30}\n", scope);
}

static void print_scope_change_v2(strview scope, FormatInfo &ffinfo)
{
	sstream ss;
	ss << "{:^" << (ffinfo.header_width) << "}";

	fmt::print(fg(fmt::terminal_color::yellow) | bg(fmt::terminal_color::bright_black) | EMPHASIS::bold, std::move(ss.str()), scope);	
	fmt::print("\n");
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

static inline std::unordered_map<str, ConfigMap*> get_mapped_config(ProfileConfig &config)
{
	return {
		{"gpkih",&Config::_conf_gpkih},
		{"pki", config.get(CONFIG_PKI)},
		{"vpn", config.get(CONFIG_VPN)}
	};
};

// input -> <file>.<section>.<key> | output -> std::tuple<str,str,str>{file,section,key}
static std::tuple<strview, strview, strview> __unpack_input(strview user_input)
{
	size_t isize = user_input.size();
	int pos_fdot = user_input.find('.');

	if(pos_fdot == -1){
		// depth 1 - just file
		return std::make_tuple(user_input, "", "");
	}

	strview file(user_input.begin(), pos_fdot);
	int pos_sdot = user_input.find('.', pos_fdot+1); 
	
	if(pos_sdot == -1){
		// depth 2 - file and section
		return std::make_tuple(file, strview(user_input.begin()+pos_fdot+1,user_input.size() - pos_fdot), "");
	}

	strview section(user_input.begin()+pos_fdot+1,pos_sdot-(pos_fdot+1));
	strview key(user_input.begin()+pos_sdot+1);

	return std::make_tuple(file,section,key);
}

// Prints requested sections | properties from config file
// called when using @<pki|vpn|gpkih>
// stops when @ is encountered, indicating next session
// @returns index of next unprocessed opt in vector, last pos when done or -1 on error
// 
static int __handle_scope_printing (strview &scope_opt, std::vector<str> &opts, int next_pos)
{
	const char *scope = scope_opt.begin()+1; // scope opt without '@'
	
	const auto [file, section, key] = __unpack_input(scope);
	

	return opts.size()-1;
}

static void __handle_gpkhi_scope_printing(std::vector<str>::iterator next, std::vector<str>::iterator end)
{
	// assume scope @gpkih
	
	auto &map = Config::_conf_gpkih;
	auto ffinfo = gpkih_default_formatinfo();
	Formatter formatter(ffinfo);
	print_scope_change_v2("gpkih",ffinfo);

	for(;next != end; ++next){
		const auto [section, key, discard] = __unpack_input(*next); 

		if(section.empty()){
			continue;
		}

		if(map.find(section.data()) == map.end()){
			PWARN("unknown section '{}'\n", section);
			fflush(stdout);
			continue;
		}

		if(key.empty()){
			// print the section
			formatter.print_headers(section);
			for(const auto &kv : map[section.data()]){
				formatter.print_keyval(kv.first,kv.second);
			}
			continue;
		}

		// check key validity
		auto &section_ref = map[section.data()];
		if(section_ref.find(key.data()) != section_ref.end()){
			PWARN("unknown key '{}' from section '{}'\n", key, section);
			continue;
		}
		// key is valid
		auto &valid_key = section_ref[key.data()];
		formatter.print_keyval(section, valid_key);
	}; 
}

// Prints every section | keyval from `gpkih.conf`
static void __handle_gpkih_full_printing()
{
	auto finfo = gpkih_default_formatinfo();
	Formatter formatter(finfo);

	print_scope_change_v2("gpkih", finfo); 
	
	ConfigMap& conf = Config::_conf_gpkih;

	for (const auto& section : conf) {
		formatter.print_headers(section.first);
		for (const auto& keyval : section.second) {
			formatter.print_keyval(keyval.first, keyval.second);
		}
	}
}

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
		__handle_gpkhi_scope_printing(opts.begin(), opts.end());
		return GPKIH_OK;
	};

	opts.erase(opts.begin());

	// Profile exists, load its config
	ProfileConfig pconfig(profile);
	if (pconfig.succesfully_loaded == false) {
		return GPKIH_FAIL;
	}

	auto scope_config_map = get_mapped_config(pconfig);
	// Iterate on args and print requested pconfiguration
	//experimental::Formatter format(experimental::Formatter::FormatInfo{10, C_ALLIGN});

	auto ffinfo = gpkih_default_formatinfo();
	Formatter formatter(ffinfo);

	if(opts.empty()){
		// ./gpkih get foo
		// profile given exists but not any specific 
		// section or property requested , print all profile
		for(const auto &kv : scope_config_map){
			print_scope_change_v2(kv.first, ffinfo);
			for(const auto &file_sections : *kv.second){
				formatter.print_headers(file_sections.first);
				for(const auto &section_kv : file_sections.second){
					formatter.print_keyval(section_kv.first, section_kv.second);
				}
			}
		}
	}

	for (int i = 0; i < opts.size(); ++i) {
		strview opt = opts[i];
		if (opt[0] == '@') {
			// Scoped printing
			__handle_scope_printing(opt,opts,i+1);
			continue;
		}

		const auto [file, section, key] = __unpack_input(opt);
		
		if(scope_config_map.find(file.data()) == scope_config_map.end()){
			PWARN("unknown file scope '{}'\n", file);
			continue; 
		}

		auto &map = *scope_config_map[file.data()];

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

		if(map.find(section.data()) == map.end()){
			PWARN("unknown section '{}' in file '{}'\n", section, file);
			continue;
		}

		auto &section_map = map[section.data()];

		// scope and section exist
		if(key.empty()){
			// print whole section
			formatter.print_headers(fmt::format("{}.{}",file,section));
			for(const auto &kv : section_map){
				formatter.print_keyval(kv.first, kv.second);
			}
			continue;	
		}

		if(section_map.find(key.data()) == section_map.end()){
			PWARN("unknown key '{}' in section '{}' from file '{}'\n", key, section, file);
			continue;
		}

		formatter.print_headers(fmt::format("{}.{}"), file, section);
		formatter.print_keyval(key, section_map[key.data()]);
		// print specific property from file's section, e.g vpn.client.remote
	}

	return GPKIH_OK;
}