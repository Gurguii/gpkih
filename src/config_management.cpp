#include "config_management.hpp"
#include "gpki.hpp"
#include <exception>
#include <filesystem>
#include <system_error>


int Config::load_file(strview path, ConfigMap &buff){
	std::ifstream file(path.data());
	if(!file.is_open()){
		seterror("couldn't open gpkih.conf '{}'",path);
		return F_NOOPEN;
	}
	str line;
	int next_section;
	while(getline(file,line)){
		char first = line[0];
		if(first == section_delim_open){
			// got a section
			// remove section delimiters and spaces from line to have the section name only 
			line.erase(std::remove_if(line.begin(),line.end(),[](char c){return empty_chars.find(c) != -1;}),line.end());
			if(buff.find(line) == buff.end()){
				PWARN("skipping unknown section '{}'\n", line);
				continue;
			}
			// got a valid section, load it
			str section_name = line;
			while(getline(file,line)){
				first = line[0];
				if(first == section_delim_open){
					file.seekg(next_section);
					break;
				}
				if(skip_chars.find(first) != -1 || line.empty()){
					continue;
				}
				sstream ss(line);
				str key,val; 
				ss >> key;
				getline(ss,val);
				val.erase(std::remove_if(val.begin(),val.end(),[](char c){return empty_chars.find(c) != -1;}),val.end());
				next_section = file.tellg();
				// got a key - value pair
				buff[section_name].emplace(key,val);
			}
		}
	}
	return GPKIH_OK;
};

/* ProfileConfig constructor */
ProfileConfig::ProfileConfig(Profile &profile, CONFIG_FILE files_to_load){
	if(files_to_load & CONFIG_PKI){
		// Load pki.conf
		auto path = fmt::format("{}{}{}",profile.source,SLASH,pki_conf_filename);
		succesfully_loaded = load_file(path,this->_conf_pki);
		try{
			// override subopts::build default params
			subopts::build::algorithm = _conf_pki["key"]["algorithm"];
			subopts::build::key_size = _conf_pki["key"]["size"];
			subopts::build::key_format = _conf_pki["key"]["creation_format"];
			subopts::build::csr_crt_format = _conf_pki["csr"]["creation_format"];
			subopts::build::days = _conf_pki["crt"]["days"];
			// override subopts::build default params
			Subject::country = _conf_pki["subject"]["country"];
			Subject::cn = _conf_pki["subject"]["common_name"];
			Subject::email = _conf_pki["subject"]["email"];
			Subject::organisation = _conf_pki["subject"]["organisation"];
			Subject::state = _conf_pki["subject"]["state"];
			Subject::location = _conf_pki["subject"]["location"];
		}catch(std::exception ex){
			PERROR("exception - {}\n", ex.what());
		}
	}
	if(files_to_load & CONFIG_VPN){
		// Load openvpn.conf
		auto path = fmt::format("{}{}{}",profile.source,SLASH,vpn_conf_filename);
		succesfully_loaded = load_file(path,this->_conf_vpn);
		// add any default param overriding required here
		try{

		}catch(std::exception ex){
			PERROR("exception - {}\n", ex.what());
		}
	}
}

ConfigMap* ProfileConfig::get(CONFIG_FILE file){
	switch(file){
		case CONFIG_VPN:
			return &this->_conf_vpn;
			break;
		case CONFIG_PKI:
			return &this->_conf_pki;
			break;
		default:
			return nullptr;
			break;
	}
}

bool ProfileConfig::exists(strview key, CONFIG_FILE file){
	auto ptr = get(file);
	if(ptr == nullptr){
		return false;
	}
	ConfigMap &conf = *ptr;
	for(auto &st : conf){
		if(st.second.find(key.data()) != st.second.end()){
			return true;
		}
	}
	return conf.find(key.data()) != conf.end();
}

static inline str skipchars = "#\n ";
bool ProfileConfig::dump_vpn_conf(strview outpath, ENTITY_TYPE type){
	if(fs::exists(outpath)){
		return -1;
	}

	std::ofstream file(outpath.data());
	if(!file.is_open()){
		seterror("couldn't open file '{}' to write",outpath);
		return F_NOOPEN;
	}
	// Dump _conf_pki map to output path
	// entity specific options
	switch(type){
		case ET_SV:
			if(this->_conf_vpn.find("server") != this->_conf_vpn.end()){
				for(auto &kv : this->_conf_vpn["server"]){
					if(kv.second == "UNSET"){
						file << "# ";
					}
					file << kv.first << " " << kv.second << "\n";
				}			
			}
			break;
		case ET_CL:
			if(this->_conf_vpn.find("client") !=  this->_conf_vpn.end()){
				for(auto &kv : _conf_vpn["client"]){
					if(kv.second == "UNSET"){
						file << "# ";	
					}
					file << kv.first << " " << kv.second << "\n";
				}
			}
			break;
		default:
			seterror("entity type '{}' not suitable for call to dump_vpn_conf()",to_str(type));
			break;
	}
	// common options
	if(this->_conf_vpn.find("common") == this->_conf_vpn.end()){
		seterror("couldn't find 'common' section in vpn mapped values");
		return GPKIH_FAIL;
	}
	for(auto &kv : this->_conf_vpn["common"]){
		if(kv.second == "UNSET"){
			file << "# ";
		}
		file << kv.first << " " << kv.second << "\n";
	}
	return (fs::exists(outpath) && fs::file_size(outpath) > 0) ? GPKIH_OK : -1;
}