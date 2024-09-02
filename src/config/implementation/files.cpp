#include "../files.hpp"
#include "../../gpkih.hpp"
#include "../../libs/printing/printing.hpp"
#include <algorithm>

using namespace gpkih;

static inline void __clear_section_line(std::string &line){
  line.erase(std::remove_if(line.begin(), line.end(), [](const char &c){return (emptyChars[0] == c || emptyChars[1] == c || emptyChars[2] == c);}), line.end());
}

int config::syncFile(std::string_view srcConfigPath, ConfigMap *cmap){
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

int config::loadFile(std::string_view path, ConfigMap &buff) {
  std::ifstream file(path.data());
  
  if (!file.is_open()) {
    PERROR("couldn't open gpkih.conf '{}'\n", path);
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

        std::stringstream ss(line);
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