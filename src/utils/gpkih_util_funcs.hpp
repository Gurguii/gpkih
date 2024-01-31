#pragma once
#include "../gpki.hpp"
#include <fstream>
#include <filesystem>
#include "../printing.hpp"
#include <unordered_map>

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

static int can_write(std::string path){
  /* this one is sad too */
  try{
    if(!fs::exists(path)){
      if(!fs::create_directory(path)){
        return -1;
      };
      return fs::remove(path);
    }
    std::string tmp = path + SLASH + "gpkih_tmpfile";
    std::ofstream(tmp,std::ios::out);
    if(!fs::exists(tmp)){
      return -1;
    }
    return fs::remove(tmp);
  }catch(fs::filesystem_error err){
    seterror(err.what());
    return -1;
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
    dstfile << std::endl;
  }
  dstfile << std::endl;
  srcfile.close();
  dstfile.close();
  return 0;
}

static int check_output_path(strview path){
  if(fs::exists(path)){
      str ans;
      PINFO("path '{}' exists, remove? y/n ");
      getline(std::cin,ans);
      for(char &c : ans){
        c = std::tolower(c);
      }
      if(ans == "n" || ans == "no"){
        return -1;
      }
      if(!fs::remove_all(path)){
        PERROR("couldn't remove '{}'\n", path);
        return -1;
      }
  }else{
    if(!fs::create_directories(path)){
      PERROR("couldn't create directory '{}'\n",path);
      return -1;
    }
  }
  return 0;
};