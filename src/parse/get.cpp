#include "parser.hpp"
#include "../experimental/formatter.hpp"

using namespace gpkih;

static void printScopeChange(strview scope, const FormatInfo &ffinfo)
{
	sstream ss;
	ss << "{:^" << (ffinfo.header_width) << "}";

	fmt::print(fg(fmt::terminal_color::yellow) | bg(fmt::terminal_color::bright_black) | EMPHASIS::bold, fmt::format("{}{}{}", "{:^",ffinfo.header_width,"}"), scope);
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

// input -> <file>.<section>.<key> | output -> std::tuple<str,str,str>{file,section,key}

/**
 * @brief      Unpacks user input "<file>.<section>.<key>"
 * 
 * @param[in]  user_input  The user input
 *
 * @return     a tuple of 3 std::string_views, they might return true on .empty() depending if file/section/key were given or just file | file/section
*/
static std::tuple<strview, strview, strview> __unpack_input(strview user_input)
{
	if(user_input[0] == '@'){
		user_input = &user_input.data()[1];
	}

	size_t isize = user_input.size();
	size_t pos_fdot = user_input.find('.');

	if(pos_fdot == -1){
		// depth 1 - just file
		return std::make_tuple(user_input, "", "");
	}
	
	strview file(&user_input[0], pos_fdot);
	size_t pos_sdot = user_input.find('.', pos_fdot+1); 
	
	if(pos_sdot == -1){
		// depth 2 - file and section
		return std::make_tuple(file, strview(&user_input[pos_fdot + 1], user_input.size() - pos_fdot - 1), "");
	}

	strview section(&user_input[pos_fdot+1], pos_sdot - (pos_fdot + 1));
	strview key(&user_input[pos_sdot+1]);

	return std::make_tuple(file,section,key);
}

// @brief prints requested sections | properties from config file, called when using @<pki|vpn> stops when @ is encountered, indicating next section
// @returns index of next unprocessed opt in vector, last pos when done or -1 on error
static size_t __handleScopePrinting (strview &scope_opt, std::vector<str> &opts, size_t next_pos, ProfileConfig& pconfig, gpkih::experimental::Formatter formatter)
{
	const char *scope = &scope_opt[1]; // scope opt without '@'
		
	const auto [file, section, key] = __unpack_input(scope);

	if(file.empty())
	{
		return next_pos;
	};

	ConfigMap *config;

	if(file == "pki")
	{
		config = pconfig.getptr(CFILE_PKI);
	}else if(file == "vpn")
	{
		config = pconfig.getptr(CFILE_VPN);

	}else {
		PWARN("unknown scope '{}'\n", file);
		return next_pos;
	}

	printScopeChange(file, formatter.get_format());
	strview opt;

	if(section.empty()){
		// iterate assuming given file scope until '@' is found
		for(size_t i = next_pos; i < opts.size(); ++i){
			opt = opts[i];
			if(opt[0] == '@'){
				//fmt::print("returning {} which is {}\n", i, opts[i]);
				// do -1 since the other loop will ++ upon iteration
				return i-1;
			}
			if(config->find(str(opt)) == config->end()){
				PWARN("section '{}' not found\n", opt);
				continue;
			}
			formatter.print_headers(opt);
			for(auto &kv : (*config)[str(opt)]){
				formatter.print_keyval(kv.first, kv.second);
			}
		} 
		return opts.size() -1;
	}	

	// got a section too, check if its a section + key or not 
	if(!key.empty()){
		PWARN("can't use scope resolutor '@' to a key, try @{}.{} {}\n",file, section, key);
		return next_pos;
	}

	std::unordered_map<str,str> *mappedSection = nullptr;

	for(auto &kv : *config){
		if(kv.first == section){
			// section exists
			mappedSection=&kv.second;
		}
	}

	if(mappedSection == nullptr){
		return next_pos;
	}

	formatter.print_headers(section);
	// iterate assuming given file + section scope, e.g @pki.key
	for(size_t i = next_pos; i < opts.size(); ++i){
		strview opt = opts[i];
		if(opt[0] == '@'){
			return i;
		}

		//fmt::print("opt: {} opt.data(): {}\n", opt, opt.data());

		/* This is not good at all since the .data() returns a c string pointer to the 
		* original string, so lets suppose_
		* original: "gurgui" strview(original,3) == "gur"
		* strview.data() -> gurgui which might lead to false negatives when doing
		* lookups on the unordered_map
		*/
		if(mappedSection->find(str(opt)) == mappedSection->end()){
			PWARN("key '{}' not found\n", opt);
			continue;
		}
		formatter.print_keyval(opt, (*mappedSection)[opt.data()]);
	}
	return opts.size();
}
// ConfigMap == std::unordered_map<str, std::unordered_map<str, str>>
// @file.section
//
/**
 * @brief      Handles calls like './gpki get <...>' where no profile is specified, assumes 'gpkih.conf'  
 * 
 * @param[in]  Iterator to the first element containing user input, e.g ./gpkih get behaviour logs -> iterator to 'behaviour'
 * @param[in]  Iterator to the end of the user input
 */
static void __handle_gpkih_scope_printing(std::vector<str>::iterator next, std::vector<str>::iterator end)
{
	// assume scope @gpkih
	auto &map = Config::gpkihConfig;
	auto ffinfo = gpkih_default_formatinfo();
	Formatter formatter(ffinfo);
	printScopeChange("gpkih",ffinfo);

	for(;next != end; ++next){
		const auto [section, key, discard] = __unpack_input(*next); 

		if(section.empty()){
			continue;
		}

		const std::unordered_map<str,str> *mappedSection = nullptr;

		for(const auto &kv : map){
			if(kv.first == section){
				mappedSection = &kv.second;
				break;
			}
		}

		if(mappedSection == nullptr){
			PWARN("section '{}' doesn't exist in scope 'gpkih'\n", section);
			return;
		}

		formatter.print_headers(section);

		if(key.empty()){
			// print the whole section
			for(const auto &kv : *mappedSection){
				formatter.print_keyval(kv.first, kv.second);
			}
			return;
		}

		// check that key exists
		auto ref = *mappedSection;
		for(const auto &kv : ref){
			if(kv.first == key){
				// key exists, print it
				formatter.print_keyval(key, ref[key.data()]);		
			}
		}
	
		return;
	}; 
}

// Prints every section | keyval from `gpkih.conf`
static void __handle_gpkih_full_printing()
{
	auto finfo = gpkih_default_formatinfo();
	Formatter formatter(finfo);

	printScopeChange("gpkih", finfo); 
	
	ConfigMap& conf = Config::gpkihConfig;

	for (const auto& section : conf) {
		formatter.print_headers(section.first);
		for (const auto& keyval : section.second) {
			formatter.print_keyval(keyval.first, keyval.second);
		}
	}
}

int parsers::get(std::vector<str> &opts)
{
	if (opts.empty()) {
		// print gpkih general configuration
		__handle_gpkih_full_printing();
		return GPKIH_OK;
	}

	// Check profile
	strview profile_name = opts[0];

	Profile profile;
	if (db::profiles::load(profile_name, profile) == GPKIH_FAIL) {
		// assumed 'profile' does not exist, so parse 
		// following opts assuming `gpkih.conf` is being
		// requested since it's the global configuration
		__handle_gpkih_scope_printing(opts.begin(), opts.end()); 		
		return GPKIH_OK;
	};

	opts.erase(opts.begin());

	// Profile exists, load its config
	ProfileConfig pconfig(profile);

	if ( (pconfig.loadedFiles() & CFILE_ALL) == false ) {
		return GPKIH_FAIL;
	}

	std::unordered_map<strview, ConfigMap*>scopeConfigMap = 
	{
		{"pki", pconfig.getptr(CFILE_PKI)},
		{"vpn", pconfig.getptr(CFILE_VPN)}
	};

	auto ffinfo = gpkih_default_formatinfo();
	Formatter formatter(ffinfo);

	if(opts.empty()){
		// ./gpkih get foo
		// @context profile given exists but not any specific section or property requested
		// @action print all profile
		for(const auto &kv : scopeConfigMap){
			printScopeChange(kv.first, ffinfo);
			for(const auto &file_sections : *kv.second){
				formatter.print_headers(file_sections.first);
				for(const auto &section_kv : file_sections.second){
					formatter.print_keyval(section_kv.first, section_kv.second);
				}
			}
		}
		return GPKIH_OK;
	}

	for (size_t i = 0; i < opts.size(); ++i) {
		strview opt = opts[i];
		//fmt::print(fg(fmt::terminal_color::bright_magenta) | EMPHASIS::bold, "{:^60}\n", fmt::format("parsing '{}'", opt));
		if (opt[0] == '@') {
			// Scoped printing
			i = __handleScopePrinting(opt, opts, i+1, pconfig,formatter);
			continue;
		}

		const auto [file, section, key] = __unpack_input(opt);

		if(scopeConfigMap.find(file) == scopeConfigMap.end()){
			PWARN("unknown file scope '{}'\n", file);
			continue; 
		}

		printScopeChange(file, ffinfo);

		auto &map = *scopeConfigMap[file];

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

		if(map.find(str(section)) == map.end()){
			PWARN("unknown section '{}' in file '{}'\n", section, file);
			continue;
		}

		auto &section_map = map[str(section)];
		// scope and section exist
		if(key.empty()){
			// print whole section
			formatter.print_headers(fmt::format("{}",section));
			for(const auto &kv : section_map){
				formatter.print_keyval(kv.first, kv.second);
			}
			continue;	
		}

		if(section_map.find(str(key)) == section_map.end()){
			PWARN("unknown key '{}' in section '{}' from file '{}'\n", key, section, file);
			continue;
		}

		formatter.print_headers(fmt::format("{}"), section);
		formatter.print_keyval(key, section_map[key.data()]);
		// print specific property from file's section, e.g vpn.client.remote
	}

	return GPKIH_OK;
}