
#include "../Config.hpp"
#include "../files.hpp"
#include "../../gpkih.hpp"
#include "../../libs/printing/printing.hpp"

using namespace gpkih;

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
    {"includedLevels",""},
    {"maxSize",""}
  }},
  {"cli",{
    {"customPS",""}
  }},
  {"formatting",{
    {"dateFormat",""}
  }}
};

int Config::load(std::string_view filepath){ 
  if(gpkihConfigPath.empty()){
    gpkihConfigPath = std::string(filepath);
    return config::loadFile(filepath, gpkihConfig);     
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

  PSUCCESS("Changed 'gpkih.{}.{}' from '{}' to '{}'\n", section, key, oval, val);  
  return GPKIH_OK;    
} // Config::set()

bool Config::section_exists(const char *section){
  DEBUG(1, "Config::section_exists()");
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
  DEBUG(1, "Config::key_exists()");
  if(gpkihConfig.find(section) != gpkihConfig.end()){
    return gpkihConfig[section].find(key) != gpkihConfig[section].end();
  }
  return false;
} // Config::key_exists

int Config::sync(){
  DEBUGF(1, "Config::sync({})", gpkihConfigPath);
  return config::syncFile(gpkihConfigPath, &gpkihConfig);
} // Config::sync

// [end] namespace Config