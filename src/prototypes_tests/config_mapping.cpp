#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <fmt/format.h>

namespace fs = std::filesystem;

#define GPKI_OK 0

enum class FILE_ERROR {
  __doesnt_exist = 2,
#define F_NOEXIST static_cast<int>(FILE_ERROR::__doesnt_exist)
  NO_READ_PERM = 4,
#define F_NOREAD
  NO_WRITE_PERM = 8,
#define F_NOWRITE
  __cant_open = 16
#define F_NOOPEN static_cast<int>(FILE_ERROR::__cant_open)
};

int map_config_file(std::string_view path,
                    std::unordered_map<std::string, std::string> &buffer,
                    char section_opening_char = '[',
                    char section_closing_char = ']', std::string skip = "#",
                    char keyval_delim = ' ', char subsection_delimiter = '.') {
  if (!fs::exists(path)) {
    return F_NOEXIST;
  }
  std::ifstream file(path.data());
  if (!file.is_open()) {
    return F_NOOPEN;
  }
  std::string line;
  size_t prevline = file.tellg();
  while (getline(file, line)) {
    if(line.empty()){
      continue;
    }
    char first = line[0], last = line[line.size() - 1];
    if(skip.find(first) != std::string::npos){
      continue;
    } 
    if (first == section_opening_char && last == section_closing_char) {
      line.erase(line.begin());
      line.erase(line.begin()+line.size()-1);
      //auto iter = std::find(line.begin(),line.end(),subsection_delimiter);
      //if(iter != line.end()){
      //  // remove the subsection to group every key-val 
      //  // into the main section's map
      //  line.erase(iter,line.end());
      //}
      line.erase(std::remove(line.begin(),line.end(),' '),line.end());
      // At this point we should have just the desired section name
      std::string sname = line;
      while (getline(file, line)) {
        first = line[0];
        if (first == section_opening_char) {
          file.seekg(prevline);
          break;
        }
        if (skip.find(first) != std::string::npos) {
          continue;
        }
        // Load section key-val to the map
        // keys will be like <section>.<key> e.g umap[client.port] = 9001
        std::stringstream ss(line);
        std::string key, val;
        getline(ss,key,keyval_delim);
        ss >> val;
        buffer.emplace(fmt::format("{}.{}",sname,key),val);
        prevline = file.tellg();
      }
    }
  }
  return GPKI_OK;
}

int main() {
  std::unordered_map<std::string, std::string> buffer;
  auto a = map_config_file("/pki/test/gpkih.conf", buffer);
  switch(a){
    case GPKI_OK:
      std::cout << "key    |    val\n";
      for(auto &kv : buffer){
        std::cout << kv.first << "          " << kv.second << "\n";
      }
      break;
    case F_NOOPEN:
    std::cerr << "couldn't find file\n";
    break;
    case F_NOEXIST:
    std::cerr << "file doesn't exist\n";
    break;
    default:
      std::cerr << "unknown error\n";
    break;
  }
  return 0;
}
