
#include "config_management.hpp"
#include <cstring>
#include <sstream>
#include <fstream>
#include <unordered_map>


static inline std::string skip_chars = "#\n ";
static inline constexpr char section_delim_open = '[';
static inline constexpr char section_delim_close = ']';
static std::string empty_chars = fmt::format("{}{} ", section_delim_open, section_delim_close);

static inline void __clear_section_line(std::string &line){
  line.erase(std::remove_if(line.begin(), line.end(), [](const char &c){return empty_chars.find(c) != -1;}), line.end());
}

using namespace gpkih;

static int load_file(fs::path path, ConfigMap &buff) {
  std::ifstream file(path);
  if (!file.is_open()) {
    seterror("couldn't open gpkih.conf '{}'", path.string());
    return F_NOOPEN;
  }
  std::string line;
  size_t next_section = file.tellg();
  while (getline(file, line)) {
    char first = line[0];
    if (first == section_delim_open) {
      // got a POSSIBLE section
      // remove section delimiters and spaces from line to 
      // have the section name only
      __clear_section_line(line);
      
      // check that section is valid
      if (buff.find(line) == buff.end()) {
        PWARN("skipping unknown section '{}'\n", line);
        continue; 
      }

      // got a valid section, load it
      std::string section_name = line;
      while (getline(file, line)) {
        first = line[0];
        if (first == section_delim_open) {
          file.seekg(next_section);
          break;
        }
        if (skip_chars.find(first) != -1 || line.empty()) {
          continue;
        }
        sstream ss(line);
        std::string key, val;
        ss >> key;
        getline(ss, val);
        // val.erase(std::remove_if(val.begin(),val.end(),[](char c){return
        // empty_chars.find(c) != -1;}),val.end());
        if (val[0] == ' ') {
          val.erase(val.begin());
        }
        next_section = file.tellg();

        // got a valid keyval pair
        buff[section_name].emplace(std::move(key), std::move(val));
      }
    }
  }
  file.close();
  return GPKIH_OK;
};

// [begin] static class Config

ConfigMap Config::_conf_gpkih = {
  {"metadata",{}},
  {"behaviour",{}},
  {"logs",{}},
  {"cli",{}},
};

int Config::load(std::string_view filepath){ 
  return load_file(filepath, _conf_gpkih); 
} // Config::load()

std::string_view Config::get(std::string_view section, std::string_view key){
  if(_conf_gpkih.find(section.data()) != _conf_gpkih.end() && _conf_gpkih.at(section.data()).find(key.data()) != _conf_gpkih.at(section.data()).end()){
    return _conf_gpkih[section.data()][key.data()];     
  }else{
    return "N/A";
  }
} // Config::get()

void Config::set(std::string_view section, std::string_view key, std::string_view val){
      _conf_gpkih[section.data()][key.data()] =  val.data();
} // Config::set()

bool Config::section_exists(const char *section){
  return _conf_gpkih.find(section) != _conf_gpkih.end();
}

const std::unordered_map<std::string,std::string>* const Config::key_exists(const char *key){
  for(const auto &section : _conf_gpkih){
    if(section.second.find(key) != section.second.end()){
      // return pointer to section that contains the key
      return &_conf_gpkih[section.first];
    }
  }
  return nullptr;
}

bool Config::key_exists(const char *section, const char *key){
  if(_conf_gpkih.find(section) != _conf_gpkih.end()){
    // section exists, return key
    return (_conf_gpkih[section].find(key) != _conf_gpkih[section].end()
      ? true
      : false);
  }
  return false;
}
// [end] namespace Config


ProfileConfig::ProfileConfig(Profile &profile, CONFIG_FILE files_to_load) {
  
  /* Load pki.conf */
  if (files_to_load & CONFIG_PKI) {  
    //auto path = std::move(fmt::format("{}{}{}", profile.source, SLASH, pki_conf_filename));
    fs::path path = profile.source;
    path /= pki_conf_filename; 
    succesfully_loaded = load_file(path, this->_conf_pki) ? false : true;
  }

  /* Load openvpn.conf */
  if (files_to_load & CONFIG_VPN) {
    //auto path = std::move(fmt::format("{}{}{}", profile.source, SLASH, vpn_conf_filename));
    fs::path path = profile.source;
    path /= vpn_conf_filename;
    succesfully_loaded = load_file(path, this->_conf_vpn) ? false : true;
  }

} // ProfileConfig::ProfileConfig()

Subject ProfileConfig::default_subject() {
  PDEBUG(1,"ProfileConfig::default_subject()");
  
  Subject subj;

  memcpy(subj.country,(_conf_pki["subject"]["country"]).data(),2);
  CALLOCATE(subj.state,reinterpret_cast<size_t*>(&subj.statelen),_conf_pki["subject"]["state"]);
  CALLOCATE(subj.location,reinterpret_cast<size_t*>(&subj.locationlen),_conf_pki["subject"]["location"]);
  CALLOCATE(subj.organisation,reinterpret_cast<size_t*>(&subj.organisationlen),_conf_pki["subject"]["organisation"]);
  CALLOCATE(subj.cn,reinterpret_cast<size_t*>(&subj.cnlen),_conf_pki["subject"]["cn"]);
  CALLOCATE(subj.email,reinterpret_cast<size_t*>(&subj.emaillen),_conf_pki["subject"]["email"]);
  PDEBUG(3,"returning default subject : [country={},state={},location={},organisation={},common_name={},email={}]", subj.country,subj.state,subj.location,subj.organisation,subj.cn,subj.email);
  
  return subj;
};

ConfigMap* const ProfileConfig::get(CONFIG_FILE file) {
  switch (file) {
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
ConfigMap& ProfileConfig::_get(CONFIG_FILE file){
  switch(file){
    case CONFIG_VPN:
      return this->_conf_vpn;
    default:
      return this->_conf_pki;
  }
}
void ProfileConfig::set(CONFIG_FILE file, std::string_view section, std::string_view key, std::string_view val){
  switch(file){
    case CONFIG_PKI:
      _conf_pki[section.data()][key.data()] = val;
      break;
    case CONFIG_VPN:
      _conf_pki[section.data()][key.data()] = val;
      break;
    default:
      break;
  }
}

bool ProfileConfig::exists(std::string_view key, CONFIG_FILE file) {
  auto ptr = get(file);
  if (ptr == nullptr) {
    return false;
  }
  ConfigMap &conf = *ptr;
  for (auto &st : conf) {
    if (st.second.find(key.data()) != st.second.end()) {
      return true;
    }
  }
  return conf.find(key.data()) != conf.end();
}


bool ProfileConfig::dump_vpn_conf(fs::path &outpath, ENTITY_TYPE type) {
  if (fs::exists(outpath)) {
    seterror("already existing file '{}'\n", outpath.string());
    return false;
  }

  std::ofstream file(outpath);
  if (!file.is_open()) {
    seterror("couldn't open file '{}' to write", outpath.string());
    return false;
  }

  // entity specific options
  switch (type) {
  case ET_SV:
    if (this->_conf_vpn.find("server") != this->_conf_vpn.end()) {
      for (auto &kv : this->_conf_vpn["server"]) {
        if (kv.second == "UNSET") {
          file << "# ";
        }
        file << kv.first << " " << kv.second << "\n";
      }
    }
    break;
  case ET_CL:
    if (this->_conf_vpn.find("client") != this->_conf_vpn.end()) {
      for (auto &kv : _conf_vpn["client"]) {
        if (kv.second == "UNSET") {
          file << "# ";
        }
        file << kv.first << " " << kv.second << "\n";
      }
    }
    break;
  default:
    seterror("entity type '{}' not suitable for call to dump_vpn_conf()",
             to_str(type));
    break;
  }
  
  // common options
  if (this->_conf_vpn.find("common") == this->_conf_vpn.end()) {
    seterror("couldn't find 'common' section in vpn mapped values");
    file.close();
    return false;
  }
  for (auto &kv : this->_conf_vpn["common"]) {
    if (kv.second == "UNSET") {
      file << "# ";
    }
    file << kv.first << " " << kv.second << "\n";
  }
  file.close();
  return (fs::exists(outpath) && fs::file_size(outpath) > 0) ? true : false;
}
// [end] class ProfileConfig