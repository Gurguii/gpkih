#include "config_management.hpp"


static inline str skip_chars = "# ";

int Config::load_file(str &&path, ConfigMap& buff){
	std::cout << "loading file " << path << "\n";
	if(!fs::exists(path)){
		return F_NOEXIST;
	}
	std::ifstream file(path);
	if(!file.is_open()){
		return F_NOOPEN;
	}
	str line,key,val;
	auto prev_line = file.tellg();
	while(getline(file,line)){
		char first = line[0];
		if(skip_chars.find(first) != -1){
			continue;
		}
		char last = line[line.size()-1];
		// valid line
		if(first == '[' && last == ']'){
			// got a section
			line.erase(line.begin());
      		line.erase(line.begin()+line.size()-1);
      		line.erase(std::remove(line.begin(),line.end(),' '),line.end());
			// removed whitespaces and brackets from line,
			// only section name left
			str section = line;
			while(getline(file,line)){
				first = line[0];
				if(skip_chars.find(first) != -1){
					continue;
				}
				if(first == '['){
					// got next section
					file.seekg(prev_line);
					break;
				}
				sstream ss(line);
				ss >> key;
				getline(ss,val);
				buff[section][key] = val;
				prev_line = file.tellg();
			}
			continue;
		}
	}
	return 0;
};

int Config::load(Profile &profile, CONFIG_FILE files_to_load){
	int rcode = -1;
	std::vector<std::future<int>> _tasks{};
	if(files_to_load & CONFIG_PKI){
		rcode = load_file(fmt::format("{}{}{}",profile.source,SLASH,pki_conf_filename), Config::_conf_pki);
	}
	if(files_to_load & CONFIG_VPN){
		rcode = load_file(fmt::format("{}{}{}",profile.source,SLASH,vpn_conf_filename), Config::_conf_vpn);
	}
	if(files_to_load & CONFIG_GPKIH){
		rcode = load_file(fmt::format("{}{}{}",profile.source,SLASH,gpkih_conf_filename), Config::_conf_gpkih);
	}
	return rcode;
}

ConfigMap* Config::get(CONFIG_FILE file){
	switch(file){
		case CONFIG_VPN:
			return &Config::_conf_vpn;
			break;
		case CONFIG_PKI:
			return &Config::_conf_pki;
			break;
		case CONFIG_GPKIH:
			return &Config::_conf_gpkih;
			break;
		default:
			return nullptr;
			break;
	}
}

bool Config::exists(strview &key, CONFIG_FILE file){
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