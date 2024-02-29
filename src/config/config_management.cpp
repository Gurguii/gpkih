
#include "config_management.hpp"
#include <sstream>
#include <fstream>
#include <unordered_map>


static inline str skip_chars = "#\n ";
static inline char section_delim_open = '[';
static inline char section_delim_close = ']';
static str empty_chars = fmt::format("{}{} ", section_delim_open, section_delim_close);

static inline void __clear_section_line(str &line){
  line.erase(std::remove_if(line.begin(), line.end(), [](const char &c){return empty_chars.find(c) != -1;}), line.end());
}

using namespace gpkih;

static int load_file(fs::path path, ConfigMap &buff) {
  std::ifstream file(path);
  if (!file.is_open()) {
    seterror("couldn't open gpkih.conf '{}'", path.string());
    return F_NOOPEN;
  }
  str line;
  ui64 next_section = file.tellg();
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
      str section_name = line;
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
        str key, val;
        ss >> key;
        getline(ss, val);
        // val.erase(std::remove_if(val.begin(),val.end(),[](char c){return
        // empty_chars.find(c) != -1;}),val.end());
        if (val[0] == ' ') {
          val.erase(val.begin());
        }
        next_section = file.tellg();

        // got a valid keyval pair
        buff[section_name].emplace(key, val);
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
};

int Config::load(strview filepath){ 
  return load_file(filepath, _conf_gpkih); 
} // Config::load()

strview Config::get(strview section, strview key){
  return _conf_gpkih[section.data()][key.data()]; 
} // Config::get()

void Config::set(strview section, strview key, strview val){
      _conf_gpkih[section.data()][key.data()] =  val.data();
} // Config::set()

void Config::print(){
  for (auto &kv : _conf_gpkih) 
  {
    fmt::print("== {} ==\n",kv.first);
    for (auto &kvv : kv.second) {
      fmt::print("{} {}\n",kvv.first, kvv.second);
    }
  }
} // Config::print()

bool Config::section_exists(const char *section){
  return _conf_gpkih.find(section) != _conf_gpkih.end();
}

const std::unordered_map<str,str>* const Config::key_exists(const char *key){
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



// [begin] class ProfileConfig

ProfileConfig::ProfileConfig(Profile &profile, CONFIG_FILE files_to_load) {
  
  if (files_to_load & CONFIG_PKI) {
    // Load pki.conf
    
    //auto path = std::move(fmt::format("{}{}{}", profile.source, SLASH, pki_conf_filename));
    fs::path path = profile.source;
    path /= pki_conf_filename; 
    succesfully_loaded = load_file(path, this->_conf_pki) ? false : true;
  }

  if (files_to_load & CONFIG_VPN) {
    // Load openvpn.conf
    
    //auto path = std::move(fmt::format("{}{}{}", profile.source, SLASH, vpn_conf_filename));
    fs::path path = profile.source;
    path /= vpn_conf_filename;
    succesfully_loaded = load_file(path, this->_conf_vpn) ? false : true;
  }
} // ProfileConfig::ProfileConfig()

void ProfileConfig::print(CONFIG_FILE files)
{
    if (files & CONFIG_VPN) {
        // print vpn config
        fmt::print("== vpn config ==\n");
        for (const auto& section : _conf_vpn) {
            for (const auto &kv : section.second) {
                fmt::print("{} -> {}\n", kv.first, kv.second);
            }
        }
    }
    if (files & CONFIG_PKI) {
        // print pki config
        fmt::print("== pki config ==\n");
        for (const auto& section : _conf_pki) {
            for (const auto& kv : section.second) {
                fmt::print("{} -> {}\n", kv.first, kv.second);
            }
        }
    }
} // ProfileConfig::print()

Subject ProfileConfig::default_subject() {
  Subject subj;
  subj.country = _conf_pki["subject"]["country"];
  subj.state = _conf_pki["subject"]["state"];
  subj.location = _conf_pki["subject"]["location"];
  subj.organisation = _conf_pki["subject"]["organisation"];
  subj.cn = "";
  subj.email = _conf_pki["subject"]["email"];
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
void ProfileConfig::set(CONFIG_FILE file, strview section, strview key, strview val){
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

bool ProfileConfig::exists(strview key, CONFIG_FILE file) {
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

// Dumps common vpn config + client|server specific configuration
// (depending on ENTITY_TYPE) to outpath.
// note: this function DOES NOT add any inlined certificate/key to the outpath
bool ProfileConfig::dump_vpn_conf(fs::path &outpath, ENTITY_TYPE type) {
  if (fs::exists(outpath)) {
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