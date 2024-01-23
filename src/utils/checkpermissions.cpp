#include "gpkih_util_funcs.hpp"

int can_write(std::string path){
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
bool hasWritePermissions(std::string dirpath) {
  /* this approach is kind of sad to see */
  try {
    auto s = std::filesystem::status(dirpath);
    return true;
  } catch (fs::filesystem_error &ex) {
    seterror(ex.what());
    return false;
  }
}
