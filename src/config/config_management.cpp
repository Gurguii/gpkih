
#include "config_management.hpp"
#include <sstream>
#include <fstream>

using namespace gpkih;
int Config::load_file(strview path, ConfigMap &buff) {
  std::ifstream file(path.data());
  if (!file.is_open()) {
    seterror("couldn't open gpkih.conf '{}'", path);
    return F_NOOPEN;
  }
  str line;
  int next_section;
  while (getline(file, line)) {
    char first = line[0];
    if (first == section_delim_open) {
      // got a section
      // remove section delimiters and spaces from line to have the section name
      // only
      line.erase(
          std::remove_if(line.begin(), line.end(),
                         [](char c) { return empty_chars.find(c) != -1; }),
          line.end());
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
        // got a key - value pair
        buff[section_name].emplace(key, val);
      }
    }
  }
  return GPKIH_OK;
};

/* ProfileConfig constructor */
ProfileConfig::ProfileConfig(Profile &profile, CONFIG_FILE files_to_load) {
  if (files_to_load & CONFIG_PKI) {
    // Load pki.conf
    auto path = fmt::format("{}{}{}", profile.source, SLASH, pki_conf_filename);
    succesfully_loaded = load_file(path, this->_conf_pki) ? false : true;
  }
  if (files_to_load & CONFIG_VPN) {
    // Load openvpn.conf
    auto path = fmt::format("{}{}{}", profile.source, SLASH, vpn_conf_filename);
    succesfully_loaded = load_file(path, this->_conf_vpn) ? false : true;
  }
}

Subject ProfileConfig::default_subject() {
    Subject subj;
    subj.country = _conf_pki["subject"]["country"];
    subj.state = _conf_pki["subject"]["state"];
    subj.location = _conf_pki["subject"]["location"];
    subj.organisation = _conf_pki["subject"]["organisation"];
    subj.cn = "";
    subj.email = _conf_pki["subject"]["email"];
    return std::move(subj);
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
bool ProfileConfig::dump_vpn_conf(strview outpath, ENTITY_TYPE type) {
  if (fs::exists(outpath)) {
    return false;
  }

  std::ofstream file(outpath.data());
  if (!file.is_open()) {
    seterror("couldn't open file '{}' to write", outpath);
    return F_NOOPEN;
  }
  // Dump _conf_pki map to output path
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
    return GPKIH_FAIL;
  }
  for (auto &kv : this->_conf_vpn["common"]) {
    if (kv.second == "UNSET") {
      file << "# ";
    }
    file << kv.first << " " << kv.second << "\n";
  }
  return (fs::exists(outpath) && fs::file_size(outpath) > 0) ? true : false;
}