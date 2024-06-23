#include "../ProfileConfig.hpp"
#include "../../gpkih.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../entities/conv.hpp"
#include "../enums.hpp"
#include "../files.hpp"

using namespace gpkih;

static ConfigMap vpn{
  {
    {"common", {
      {"tls-auth",""},
      {"dev",""},
      {"cipher",""},
      {"proto",""}
    }}, // common vpn config will be mapped here as key-value strings
    
    {"client", {
      {"verb",""},
      {"installdir",""},
      {"remote-cert-tls",""},
      {"remote",""}
    }}, // * * client
    
    {"server", {
      {"push",""},
      {"ifconfig-pool-persist",""},
      {"dh",""},
      {"verb",""},
      {"crl-verify",""},
      {"keepalive",""},
      {"status",""},
      {"explicit-exit-notify",""},
      {"port",""},
      {"server",""}
    }}, // * * server
  }
};

static ConfigMap pki{
  {
    {"key", {
      {"size",""},
      {"algorithm",""}
    }},    // pki key config will be mapped here
  
    {"crt", {
      {"days",""},
    }},    // * * certificate
  
    {"subject",{
      {"email",""},
      {"common_name",""},
      {"organisation",""},
      {"location",""},
      {"state",""},
      {"country",""}
    }}, // * * subject defaults (country, state, location, organisation, email)
    
    {"output",{
      {"create_pfx",""},
      {"create_inline",""}
    }},  // certain behaviour when building certificates (create_inline, create_pfx)  
  }
};

ProfileConfig::ProfileConfig(Profile &prof, CONFIG_FILE filesToLoad)
:profile(prof),vpnConfig(vpn),pkiConfig(pki){
  fs::path path = profile.source;
  vpnConfigPath = path/vpnConfFilename;
  pkiConfigPath = path/pkiConfFilename;

  /* Load pki.conf */
  if (filesToLoad & CFILE_PKI && config::loadFile(pkiConfigPath.c_str(), this->pkiConfig) == GPKIH_OK) {
    succesfullyLoadedFiles = succesfullyLoadedFiles | CFILE_PKI;
  }

  /* Load openvpn.conf */
  if (filesToLoad & CFILE_VPN && config::loadFile(vpnConfigPath.c_str(), this->vpnConfig) == GPKIH_OK) {
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
  ADD_LOG(LL_INFO, fmt::format("profile:{} action:set {}", this->profile.name, msg));
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
             gpkih::entity::conversion::toString(type));
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
    if(config::syncFile(vpnConfigPath.c_str(), &vpnConfig) == GPKIH_FAIL){
      return GPKIH_FAIL;
    }
  }

  if (files & CFILE_PKI){
    PDEBUG(2, "syncing PKI - {}\n", pkiConfigPath.string());
    if(config::syncFile(pkiConfigPath.c_str(), &pkiConfig) == GPKIH_FAIL){
      return GPKIH_FAIL;
    }
  }  
  
  return GPKIH_OK;
};
// [end] class ProfileConfig