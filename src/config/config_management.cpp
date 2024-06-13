
#include "config_management.hpp"
#include <cstdio>
#include <cstring>
#include <sstream>
#include <fstream>
#include "../printing/printing.hpp"
#include "../memory/memmgmt.hpp"

static inline constexpr char sectionOpenDelim = '[';
static inline constexpr char sectionCloseDelim = ']';
static inline constexpr const char *emptyChars = "[] ";
static inline constexpr const char *gpkihSections = "logs behaviour cli formatting";
static inline constexpr const char *skipChars = "#\n ";

static inline void __clear_section_line(std::string &line){
  line.erase(std::remove_if(line.begin(), line.end(), [](const char &c){return (emptyChars[0] == c || emptyChars[1] == c || emptyChars[2] == c);}), line.end());
}

using namespace gpkih;

static int __syncFile(std::string_view srcConfigPath, ConfigMap *cmap){
  std::string tmpPath = std::string{srcConfigPath}+".new";

  std::ifstream src{srcConfigPath.data()};
  std::ofstream dst{tmpPath};

  if(!src.is_open()){
    PERROR("Couldn't open old configuration file '{}' for reading\n", srcConfigPath);
    return GPKIH_FAIL;
  }

  if(!dst.is_open()){
    PERROR("Couldn't open new configuration file '{}' for writing\n", tmpPath);
    return GPKIH_FAIL;
  }

  std::string line{};
  bool exists = false;

  while(getline(src, line)){

    if(line.empty()){
      continue;
    }
    
    if(line[0] == '#'){
      dst << line << EOL;
      continue;
    }

    if(line[0] != sectionOpenDelim || line.empty()){
      continue;
    }

    __clear_section_line(line);
    
    if(cmap->find(line) == cmap->end()){
      PWARN("Skipping unexistant section '{}'\n", line);
      size_t pos = src.tellg();
      while(getline(src, line)){
        if(line[0] == sectionOpenDelim){
          src.seekg(pos);
          break;
        }
        pos = src.tellg();
      }
      continue;
    }

    // Add [ sectionName ] to the file indicating the beginning of the section
    dst << sectionOpenDelim << " " << line << " " << sectionCloseDelim << EOL;
    auto &section = (*cmap)[line];
    size_t pos = src.tellg();
    
    while(getline(src, line)){
      
      if(line.empty()){
        continue;
      }

      if(line[0] == '#'){
        dst << line << EOL;
        continue;
      }

      if(line[0] == sectionOpenDelim){
        src.seekg(pos);
        break;
      }

      std::stringstream ss{line};
      std::string key, val;
      ss >> key;
      ss >> val;
      exists = false; 
      
      if(section.find(key) == section.end()){
        PWARN("Skipping unexistant key '{}'\n", key);
        continue;
      }else{
        dst << key << ' ' << section[key] << EOL;
      }
      
      pos=src.tellg();
    }
  }

  src.close();
  dst.close();

  if(fs::remove(srcConfigPath) == false){
    PWARN("Wrote new configuration to '{}' but couldn't delete older config file '{}'\n", tmpPath, srcConfigPath);
    return GPKIH_FAIL;
  };

  fs::rename(tmpPath,srcConfigPath);
  
  return fs::exists(srcConfigPath) == true ? GPKIH_OK : GPKIH_FAIL;

}

static int __load_file(fs::path path, ConfigMap &buff) {
  std::ifstream file(path);
  
  if (!file.is_open()) {
    PERROR("couldn't open gpkih.conf '{}'", path.string());
    return GPKIH_FAIL;
  }

  std::string line;
  size_t nextSection = file.tellg();
  
  while (getline(file, line)) {
    char first = line[0];
    if (first == sectionOpenDelim) {
      // got a POSSIBLE section
      // remove section delimiters and spaces from line to 
      // have the section name only
      __clear_section_line(line);
      
      // check that section is valid
      if (buff.find(line) == buff.end()) {
        PWARN("Skipping unknown section '{}'\n", line);
        size_t pos = file.tellg();
        while(getline(file, line)){
          if(line[0] == sectionOpenDelim){
            file.seekg(pos);
            break;
          }
          pos=file.tellg();
        }
        continue; 
      }

      // got a valid section, load it
      std::string sectionName = line;
      auto &sectionMap = buff[sectionName];

      while (getline(file, line)) {
        first = line[0];
 
        if (first == sectionOpenDelim) {
          file.seekg(nextSection);
          break;
        }
        if ( (skipChars[0] == first || skipChars[1] == first || skipChars[2] == first) || line.empty()) {
          continue;
        }

        sstream ss(line);
        std::string key, val;
        ss >> key;
        getline(ss, val);

        if (val[0] == ' ') {
          val.erase(val.begin());
        }
        nextSection = file.tellg();

        if(sectionMap.find(key) == sectionMap.end()){
          PWARN("Skipping invalid key '{}.{}' loading configuration\n", sectionName, key);
          continue;
        }

        sectionMap[key]=val;

      }
    }
  }
  file.close();
  return GPKIH_OK;
};

// [begin] static class Config
static inline std::string gpkihConfigPath;

ConfigMap Config::gpkihConfig = {
  {"behaviour",{
    {"headers",""},
    {"print_generated_certificate",""},
    {"prompt",""},
    {"autoanswer",""}
  }},
  {"logs",{
    {"includedFormatFields",""},
    {"includedLogLevels",""},
    {"max_size",""}
  }},
  {"cli",{
    {"customPS",""}
  }},
  {"formatting",{
    {"date_format",""}
  }}
};

int Config::load(std::string_view filepath){ 
  if(gpkihConfigPath.empty()){
    gpkihConfigPath = std::string(filepath);
    return __load_file(filepath, gpkihConfig);     
  }
  
  return GPKIH_FAIL;
} // Config::load()

std::string_view Config::get(std::string_view section, std::string_view key){
  if(gpkihConfig.find(section.data()) != gpkihConfig.end() && gpkihConfig.at(section.data()).find(key.data()) != gpkihConfig.at(section.data()).end()){
    return gpkihConfig[section.data()][key.data()];     
  }else{
    return "N/A";
  }
} // Config::get()

int Config::set(std::string_view section, std::string_view key, std::string_view val){
  if(gpkihConfig.find(section.data()) == gpkihConfig.end()){
    PERROR("Section '{}' doesn't exist, valid sections: {}\n", section, gpkihSections);
    return GPKIH_FAIL;
  }

  auto &ref = gpkihConfig[section.data()];

  if(ref.find(key.data()) == ref.end()){
    std::stringstream msg{};
    msg << "Key '" << key << "' doesn't exist in section '" << section << "', valid keys: ";
    for(const auto &kv : ref){
      msg << kv.first << " ";
    }
    PERROR("{}\n", msg.str());
    return GPKIH_FAIL;
  }

  std::string oval{gpkihConfig[section.data()][key.data()]};
  gpkihConfig[section.data()][key.data()] =  val.data();

  PSUCCESS("Changed 'gpkih.{}.{}' from '{}' to '{}\n", section, key, oval, val);  
  return GPKIH_OK;    
} // Config::set()

bool Config::section_exists(const char *section){
  PDEBUG(1, "Config::section_exists()");
  return gpkihConfig.find(section) != gpkihConfig.end();
} // Config::section_exists()

const std::unordered_map<std::string,std::string>* const Config::key_exists(const char *key){
  for(const auto &section : gpkihConfig){
    if(section.second.find(key) != section.second.end()){
      // return pointer to section that contains the key
      return &gpkihConfig[section.first];
    }
  }
  return nullptr;
} // Config::key_exists

bool Config::key_exists(const char *section, const char *key){
  PDEBUG(1, "Config::key_exists()");
  if(gpkihConfig.find(section) != gpkihConfig.end()){
    return gpkihConfig[section].find(key) != gpkihConfig[section].end();
  }
  return false;
} // Config::key_exists

int Config::sync(){
  PDEBUG(1, "Config::sync({})", gpkihConfigPath);
  return __syncFile(gpkihConfigPath, &gpkihConfig);
} // Config::sync

// [end] namespace Config

ProfileConfig::ProfileConfig(Profile &prof, CONFIG_FILE filesToLoad):profile(prof){
  fs::path path = profile.source;
  vpnConfigPath = path/vpnConfFilename;
  pkiConfigPath = path/pkiConfFilename;

  /* Load pki.conf */
  if (filesToLoad & CFILE_PKI && __load_file(pkiConfigPath, this->pkiConfig) == GPKIH_OK) {
    succesfullyLoadedFiles = succesfullyLoadedFiles | CFILE_PKI;
  }

  /* Load openvpn.conf */
  if (filesToLoad & CFILE_VPN && __load_file(vpnConfigPath, this->vpnConfig)) {
    succesfullyLoadedFiles =  succesfullyLoadedFiles | CFILE_VPN;
  }
} // ProfileConfig::ProfileConfig()

Subject ProfileConfig::default_subject() {
  PDEBUG(1,"ProfileConfig::default_subject()");
  
  Subject subj;

  memcpy(subj.country,(pkiConfig["subject"]["country"]).data(),2);
  CALLOCATE(subj.state,reinterpret_cast<size_t*>(&subj.statelen),pkiConfig["subject"]["state"]);
  CALLOCATE(subj.location,reinterpret_cast<size_t*>(&subj.locationlen),pkiConfig["subject"]["location"]);
  CALLOCATE(subj.organisation,reinterpret_cast<size_t*>(&subj.organisationlen),pkiConfig["subject"]["organisation"]);
  CALLOCATE(subj.cn,reinterpret_cast<size_t*>(&subj.cnlen),pkiConfig["subject"]["cn"]);
  CALLOCATE(subj.email,reinterpret_cast<size_t*>(&subj.emaillen),pkiConfig["subject"]["email"]);

  return subj;
};

ConfigMap* const ProfileConfig::getptr(CONFIG_FILE file) {
  PDEBUG(1, "ProfileConfig::getptr()");

  switch (file) {
  case CFILE_VPN:
    return &this->vpnConfig;
    break;
  case CFILE_PKI:
    return &this->pkiConfig;
    break;
  default:
    return nullptr;
  }
}

ConfigMap& ProfileConfig::get(CONFIG_FILE file){
  PDEBUG(1, "ProfileConfig::get()");

  switch(file){
    case CFILE_VPN:
      return this->vpnConfig;
    default:
      return this->pkiConfig;
  }
}

const CONFIG_FILE ProfileConfig::loadedFiles(){
  PDEBUG(1, "ProfileConfig::loadedFiles()");

  return this->succesfullyLoadedFiles; 
}

int ProfileConfig::set(CONFIG_FILE file, std::string_view section, std::string_view key, std::string_view val){
  PDEBUG(1, "ProfileConfig::set()");

  try{
    if(file & CFILE_PKI){
      pkiConfig.at(section.data()).at(key.data()) = val;
    }else if(file & CFILE_VPN){
      vpnConfig.at(section.data()).at(key.data()) = val;
    }else{
      return GPKIH_FAIL;
    }
    return GPKIH_OK;
  }catch(const std::out_of_range &err){
    PERROR(err.what());
    return GPKIH_FAIL;
  }
}

int ProfileConfig::set2(CONFIG_FILE file, std::string_view section, std::string_view key, std::string_view val){
  PDEBUG(1, "ProfileConfig::set2()");

  try{
    if(file & CFILE_PKI){
      pkiConfig.at(section.data()).at(key.data()) = val;
    }else if(file & CFILE_VPN){
      vpnConfig.at(section.data()).at(key.data()) = val;
    }else{
      return GPKIH_FAIL;
    }
  }catch(const std::out_of_range &err){
    if(file & CFILE_PKI){
      if(pkiConfig.find(section.data()) == pkiConfig.end()){
        PERROR("Section '{}' doesn't exist in pki file\n", section.data());
      }else{
        PERROR("Key '{}' doesn't exist in pki file section '{}'\n", key.data(), section.data());
      }
    }else if(file & CFILE_VPN){
      if(vpnConfig.find(section.data()) == vpnConfig.end()){
        PERROR("Section '{}' doesn't exist in vpn file\n", section.data());
      }else{
        PERROR("Key '{}' doesn't exist in vpn file section '{}'\n", key.data(), section.data());
      }      
    }
    return GPKIH_FAIL;
  }

  std::string msg = fmt::format("changed {}.{}.{} to '{}'", file == CFILE_PKI ? "pki" : "vpn", section, key, val);
  PSUCCESS("{}\n", msg);
  ADD_LOG(L_INFO, fmt::format("profile:{} action:set {}", this->profile.name, msg));
  return GPKIH_OK;
}

bool ProfileConfig::key_exists(std::string_view key, CONFIG_FILE file) {
  PDEBUG(1, "ProfileConfig::key_exists()");

  auto ptr = getptr(file);
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

bool ProfileConfig::key_exists(CONFIG_FILE file, std::string_view section, std::string_view key){
  PDEBUG(1, "ProfileConfig::key_exists()");

  std::unordered_map<std::string, std::string> *sptr;

  if(section_exists(section, file) == false){
    return false;
  }

  switch(file){
    case CFILE_PKI:
      sptr = &pkiConfig[section.data()];
      break;
    case CFILE_VPN:
      sptr = &vpnConfig[section.data()];
      break;
    default:
      return false;    
  }
  
  return sptr->find(key.data()) != sptr->end();
}

bool ProfileConfig::section_exists(std::string_view section, CONFIG_FILE file){
  PDEBUG(1, "ProfileConfig::section_exists()");

  auto ptr = getptr(file);
  if(ptr == nullptr){
    return false;
  }
  return ptr->find(section.data()) != ptr->end();
}

bool ProfileConfig::dump_vpn_conf(fs::path &outpath, ENTITY_TYPE type) {
  PDEBUG(1, "ProfileConfig::dump_vpn_conf()");

  if (fs::exists(outpath)) {
    PERROR("already existing file '{}'\n", outpath.string());
    return false;
  }

  std::ofstream file(outpath);
  if (!file.is_open()) {
    PERROR("couldn't open file '{}' to write", outpath.string());
    return false;
  }

  // entity specific options
  switch (type) {
  case ET_SV:
    if (this->vpnConfig.find("server") != this->vpnConfig.end()) {
      for (auto &kv : this->vpnConfig["server"]) {
        if (kv.second == "UNSET") {
          file << "# ";
        }
        file << kv.first << " " << kv.second << "\n";
      }
    }
    break;
  case ET_CL:
    if (this->vpnConfig.find("client") != this->vpnConfig.end()) {
      for (auto &kv : vpnConfig["client"]) {
        if (kv.second == "UNSET") {
          file << "# ";
        }
        file << kv.first << " " << kv.second << "\n";
      }
    }
    break;
  default:
    PERROR("entity type '{}' not suitable for call to dump_vpn_conf()",
             to_str(type));
    break;
  }
  
  // Common options
  if (this->vpnConfig.find("common") == this->vpnConfig.end()) {
    PERROR("couldn't find 'common' section in vpn mapped values");
    file.close();
    return false;
  }
  for (auto &kv : this->vpnConfig["common"]) {
    if (kv.second == "UNSET") {
      file << "# ";
    }
    file << kv.first << " " << kv.second << "\n";
  }
  file.close();
  return (fs::exists(outpath) && fs::file_size(outpath) > 0) ? true : false;
}

int ProfileConfig::sync(CONFIG_FILE files){
  PDEBUG(1, "ProfileConfig::sync()");
  
  if (files & CFILE_VPN){
    PDEBUG(2, "syncing VPN - {}\n", vpnConfigPath.string());
    if(__syncFile(vpnConfigPath.c_str(), &vpnConfig) == GPKIH_FAIL){
      return GPKIH_FAIL;
    }
  }

  if (files & CFILE_PKI){
    PDEBUG(2, "syncing PKI - {}\n", pkiConfigPath.string());
    if(__syncFile(pkiConfigPath.c_str(), &pkiConfig) == GPKIH_FAIL){
      return GPKIH_FAIL;
    }
  }  
  
  return GPKIH_OK;
};
// [end] class ProfileConfig