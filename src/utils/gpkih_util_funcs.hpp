#pragma once
#include "../gpki.hpp"
#include <fstream>
#include <filesystem>
#include "../printing.hpp"
#include <unordered_map>
#include <sstream>
static bool hasWritePermissions(std::string dirpath) {
  /* this approach is kind of sad to see */
  try {
    auto s = std::filesystem::status(dirpath);
    return true;
  } catch (fs::filesystem_error &ex) {
    seterror(ex.what());
    return false;
  }
}

/* sed("/home/gurgui/base", "/home/gurgui/aftersed.txt",
          {{"GPKI_BASEDIR", "WISKONSIN"}})
*/
static int sed(std::string_view src, std::string_view dst,
  std::unordered_map<std::string_view, std::string_view> vals) {
  std::ifstream srcfile(src.data());
  if (!srcfile.is_open()) {
    return -1;
  }
  std::ofstream dstfile(dst.data());
  if (!dstfile.is_open()) {
    return -1;
  }
  std::string line;
  std::string word;
  while (getline(srcfile, line)) {
    auto ss = std::stringstream(line);
    while (ss >> word) {
      if (vals.find(word) != vals.end()) {
        dstfile << vals[word];
      } else {
        dstfile << word;
      }
      dstfile << " ";
    }
    dstfile << EOL;
  }
  dstfile << EOL;
  srcfile.close();
  dstfile.close();
  return 0;
}

// Attempts to create the target path (directory)
// - checks if a file already exists
// - creates every non-existant intermediary directory
static int create_output_path(str &path){
  if(fs::exists(path)){
    str ans;
    PROMPT("path '" + path + "'exists, remove?","[y/n]");
    getline(std::cin,ans);
    for(char &c : ans){
      c = std::tolower(c);
    }
    if(ans == "n" || ans == "no"){
      return -1;
    }
    try{
      fs::remove_all(path);
    }catch(fs::filesystem_error err){
      PERROR(err.what());
      return -1;
    }
  }else{
    if(!fs::create_directories(path)){
      PERROR("couldn't create directory '{}'\n",path);
      return -1;
    }
  }
  // Directory succesfully created
  return 0;
};
static int check_out_file(str _path){
  if(fs::exists(_path)){
      str ans;
      PINFO("path '{}' exists, remove? y/n ");
      getline(std::cin,ans);
      for(char &c : ans){
        c = std::tolower(c);
      }
      if(ans == "n" || ans == "no"){
        return -1;
      }
      if(!fs::remove_all(_path)){
        PERROR("couldn't remove '{}'\n", _path);
        return -1;
      }
  }else{
    if(std::ofstream(_path).is_open()){
      PERROR("couldn't create directory '{}'\n",_path);
      return -1;
    }
  }
  return 0;
};
