#pragma once
#include "../gpki.hpp" // #include <sstream> + typename aliases (str, strview ...) 
#include <unordered_map>
#include <fstream> // std::ifstream | std::ofstream
#include <iostream> // std::cin
#include <sstream> 

static bool hasWritePermissions(std::string dirpath) {
  /* this approach is kind of sad to see */
  try {
    auto s = fs::status(dirpath);
    return true;
  } catch (fs::filesystem_error &ex) {
    seterror(ex.what());
    return false;
  }
}

/* sed("/home/gurgui/base", "/home/gurgui/aftersed.txt",
          {{"GPKI_BASEDIR", "WISKONSIN"}})
*/
static int sed(strview src, strview dst,
  std::unordered_map<strview, strview> &&vals) {
  std::ifstream srcfile(src.data());
  if (!srcfile.is_open()) {
    return -1;
  }
  std::ofstream dstfile(dst.data());
  if (!dstfile.is_open()) {
    return -1;
  }
  str line;
  str word;
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

// Returns std::string with the filename
// of given path
static str filename_from_path(strview path){
  auto iter = std::find(path.rbegin(),path.rend(),SLASH);
  if(*iter == SLASH){

  }
  str ss;
  while(iter != path.rbegin()){
    ss += *iter--;
  } 
  ss += *iter;
  return std::move(ss);
}

// Attempts to create the target path (directory or file)
// - if dir != 0 a directory is created and any missing intermediary
// directories will be created too, else a file will be created
static int create_output_path(fs::path &path, int dir){
  if(path.is_relative()){
    seterror("path to create_output_path() must be absolute, given path: {}", path.string());
    return GPKIH_FAIL;
  }

  if(fs::exists(path)){
    str ans;
    PROMPT("path '" + path.string() + "'exists, remove?","[y/n]");
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
    if(dir){
      if(!fs::create_directories(path)){
        PERROR("couldn't create directory '{}'\n",path.string());
        return GPKIH_FAIL;
      }
    }else{
      if(std::ofstream(path).is_open()){
        return GPKIH_OK;
      }      
      return GPKIH_OK;
    }
  }
  // Directory succesfully created
  return GPKIH_OK;
};