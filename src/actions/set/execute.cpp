#include "ASet.hpp"

#include "../../libs/printing/printing.hpp"

#include "../../config/ProfileConfig.hpp"
#include "../../config/Config.hpp"

#include "../../db/profiles.hpp"

#include "../../gpkih.hpp"

#include <variant>

using namespace gpkih;

size_t __handleScopedProfile(std::vector<std::string>::iterator &iter, std::vector<std::string>::iterator end, ProfileConfig &pconf, CONFIG_FILE &alteredFiles){
	DEBUG(2, "__handleScopedProfile()");

	std::string scfile;
	CONFIG_FILE cfile;
	std::string section;
	ConfigMap *mappedConf;

	int depth = std::count(iter->begin(), iter->end(), '.');

	if(depth == 0){
		// @pki key.size=1024 client.remote='192.168.1.1'
		scfile = std::string{iter->c_str()+1};
	}else if(depth == 1){
		// @pki.key size=1024 ...
		auto dotpos = iter->find_first_of('.');
		if(dotpos == -1){
			return GPKIH_FAIL;
		}
		scfile = std::string{iter->begin() + 1, iter->begin() + dotpos};
		section = std::string{iter->begin() + dotpos + 1, iter->end()};
	}else{
		PWARN("Skipping wrongly formatted scope resolution '{}'\n", iter->c_str());
		PHINT("Scope resolution can be achieved by doing:\n  1. @file.section key=nval [...]\n  2. @file section.key=nval [...]\nNote the dot '.' positioning\n");
		return GPKIH_FAIL;
	}

	if(scfile == "pki"){
		cfile = CFILE_PKI;	
	}else if(scfile == "vpn"){
		cfile = CFILE_VPN;
	}else{
		PWARN("Invalid configuration file '{}' not among valid options: pki|vpn\n", scfile.c_str());
		while(iter != end && iter->c_str()[0] != '@'){
			++iter;
		}
		return GPKIH_FAIL;
	}

	mappedConf = pconf.getptr(cfile);
	
	if(depth == 0){
		// ./gpkih set foo @pki key.size=1024 ...
		while(++iter != end && (*iter)[0] != '@'){
			std::stringstream ss(*iter);
			std::string section, key, nval;
			getline(ss, section, '.');
			getline(ss, key, '=');
			getline(ss, nval);

			if(section.empty() || key.empty() || nval.empty()){
				PWARN("Invalid syntax '{}', in this context it must be 'section.name=nval'\n", *iter);
				continue;
			}

			std::string oval{(*mappedConf)[section][key]};

			if(pconf.set2(cfile, section, key, nval) == GPKIH_FAIL){
				continue;
			};

			alteredFiles = alteredFiles | cfile;
		}
	}else if(depth == 1){
		// ./gpkih foo @vpn.client remote='1.1.1.1 9999' ...
		if(pconf.section_exists(section, cfile) == false){
			PWARN("Section '{}' doesn't exist\n", section);
			// Skip until end of vector or next @
			while(++iter != end && (*iter)[0] != '@'){
				PHINT("Skipping {}\n", *iter);
			}
			--iter;
			return GPKIH_OK;
		}

		auto &mappedSection = (*mappedConf)[section.data()];

		for(; ++iter != end && (*iter)[0] != '@'; ){
			std::string key{};
			std::string nval{};
			std::stringstream ss{*iter};
			getline(ss, key, '=');
			getline(ss, nval);

			if(mappedSection.find(key) == mappedSection.end()){
				// Key doesn't exist
				PWARN("Omitting unexistant key '{}.{}.{}'\n", scfile, section, key);
				continue;
			}

			if(pconf.set2(cfile, section, key, nval) == GPKIH_FAIL){
				return GPKIH_FAIL;
			};

			alteredFiles = alteredFiles | cfile;
		}

		if(iter != end){
			// A '@' was found so we need to go back so that when the calling function's for loop increments it can detect the @ and handle scope resolution
			--iter;
		}
		// TODO
		// Start iterating on key=val with chosen cfile + section, check key existance and set new value
	}
 
	return GPKIH_OK;
}

int __handleGpkih(std::vector<std::string> &opts){
	DEBUG(1, "__handleGpkih()");

	const char *section = nullptr;
	for(auto current = opts.begin(); current != opts.end(); ++current){
		if((*current)[0] == '@'){
			section = current->c_str()+1;

			for(auto keyval = ++current; keyval != opts.end(); ++keyval){
				
				if((*keyval)[0] == '@'){
					current = keyval-1;
					break;
				}
				
				std::stringstream ss(*keyval);
				std::string key,nval;
				getline(ss, key, '=');
				getline(ss, nval);
				
				if(key.empty()){
					PWARN("key is empty '{}'\n", key);
					return GPKIH_FAIL;
				}else if(nval.empty()){
					PWARN("nval is empty '{}'\n", nval);
					return GPKIH_FAIL;
				}

				std::string oval{Config::get(section, key)};

				if(Config::set(section, key, nval) == GPKIH_FAIL){
					return GPKIH_FAIL;
				}
			}
			
			break;				
		}else{
			int dots = std::count(current->begin(), current->end(), '.');
			
			if(dots != 1){
				PWARN("In this context (global gpkih configuration) the syntax must be 'section.key=nval'\n");
				PINFO("Current: {}\n", *current);
				continue;
			}
			
			std::stringstream ss{*current};
			std::string key,section,nval;
			getline(ss, section, '.');
			getline(ss, key, '=');
			getline(ss, nval);
			
			std::string oval{};

			if(Config::set(section, key, nval) == GPKIH_FAIL){
				return GPKIH_FAIL;
			}
		}
	}
	if(DRY_RUN == false){
		// Sync config to update changes
		return Config::sync();
	}else{
		return GPKIH_OK;
	}
}

int __handleProfile(std::vector<std::string> &opts){
	DEBUG(2, "__handleProfile()");

	Profile profile;
	CONFIG_FILE alteredFiles = CFILE_NONE;

	if(db::profiles::load(opts[0], profile) == GPKIH_FAIL){
		PWARN("Profile '{}' doesn't exist\n", opts[0]);
		return GPKIH_FAIL;
	}

	if(opts.size() == 1){
		PHINT("Profile '{}' exists but no args given, try ./gpkih help set\n", profile.name);
		return GPKIH_FAIL;
	}

	opts.erase(opts.begin());
	ProfileConfig pconf(profile);
	opts.erase(std::remove_if(opts.begin(), opts.end(), [](std::string &opt){return opt.empty();}),opts.end());
	
	for(auto iter = opts.begin(); iter != opts.end(); ++iter){
		if(iter->empty()){
			continue;
		}
		
		std::string_view opt = *iter;		
		
		if(opt[0] == '@'){
			// Scope resolution e.g ./gpkih set foo @vpn.client remote='192.168.1.1 9999' ...
			__handleScopedProfile(iter, opts.end(), pconf, alteredFiles);
			if(iter == opts.end()){
				break;
			}
			continue;
		}

		// No scope resolution e.g ./gpkih set foo vpn.client.remote='192.168.1.1 9999' ...
		if(opt.find('=') == std::variant_npos){
			PWARN("Invalid keyval syntax '{}'\n", opt);
			PHINT("Keyval syntax - cfile.section.key=nval (note the equal '=' sign)\n");
			continue;
		}

		std::stringstream ss {opt.data()};
		std::string scope{}; std::string cfilestr{}; std::string section{}; std::string key{}; std::string nval{};

		getline(ss, scope, '=');
		std::stringstream sscope(scope);

		getline(sscope, cfilestr, '.');
		getline(sscope, section, '.');
		getline(sscope, key);

		if(section == "subject" && key == "email" && std::count(ss.str().begin(), ss.str().end(), '.') != 2 ){
			PWARN("Invalid syntax '{}', in this context it must be <pki|vpn>.section.key=nval\n", *iter);
			continue;
		}

		getline(ss, nval);

		CONFIG_FILE cfile;
		ConfigMap *cptr;

		if(cfilestr == "vpn"){
			cfile = CFILE_VPN;
		}else if(cfilestr == "pki"){
			cfile = CFILE_PKI;
		}else{
			PERROR("Invalid configuration file '{}' not among: vpn | pki\n", cfilestr);
			continue;
		}

		cptr = pconf.getptr(cfile);
		if(cptr == nullptr){
			return GPKIH_FAIL;
		}

		if(pconf.set2(cfile, section, key, nval) == GPKIH_FAIL){
			continue;
		};

		alteredFiles = alteredFiles | cfile;
	}

	if(DRY_RUN == false){
		// Sync config to update changes
		if(alteredFiles & CFILE_PKI && pconf.sync(CFILE_PKI) == GPKIH_FAIL){
			return GPKIH_FAIL;
		}
		if(alteredFiles & CFILE_VPN && pconf.sync(CFILE_VPN) == GPKIH_FAIL){
			return GPKIH_FAIL;
		}
		return GPKIH_OK;
	}

	return GPKIH_OK;
}

int ASet::exec(std::vector<std::string> &args) const {
	/* BEG - Parse args */
	DEBUG(1,"parsers::set()");

	// ./gpkih set @behaviour
	if(args.size() == 0){
		PHINT("No args given, try ./gpkih help set\n");
		return GPKIH_FAIL;
	}

	if(args[0][0] == '@' || std::count(args[0].begin(), args[0].end(), '.') > 0){
		return __handleGpkih(args);
	}

	return __handleProfile(args);
	/* END - Parse args */
}