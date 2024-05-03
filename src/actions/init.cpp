#include "actions.hpp"
#include <fmt/color.h>
#include <iostream> // std::cin
#include <fstream>
#include <sstream>
static inline std::string openssl_conf_filename = "gopenssl.conf";

static inline std::vector<std::string> RELATIVE_DIRECTORY_PATHS(){
  return 
  {
    "packs",
    "logs",
    fmt::format("pki{}ca", SLASH),
    fmt::format("pki{}certs", SLASH),
    fmt::format("pki{}keys", SLASH),
    fmt::format("pki{}reqs", SLASH),
    fmt::format("pki{}serial", SLASH),
    fmt::format("pki{}database", SLASH),
    fmt::format("pki{}crl", SLASH),
    fmt::format("pki{}tls",SLASH)                                                                                       
  };
}

static inline std::unordered_map<std::string,std::string> RELATIVE_FILE_PATHS(){
  return 
  {  
    {fmt::format("pki{}crl{}crlnumber", SLASH, SLASH), "1000"},
    {fmt::format("pki{}database{}index.txt", SLASH, SLASH), ""},
    {fmt::format("pki{}serial{}serial", SLASH, SLASH), "01"}                                                         
  };
}  

static bool __has_write_perms(std::string dirpath) {
  PDEBUG(2, "__has_write_perms()");

  /* this approach is kind of sad to see */
  try {
    auto s = fs::status(dirpath);
    return true;
  } catch (fs::filesystem_error &ex) {
    seterror(ex.what());
    return false;
  }
}

/* __sed("/home/gurgui/base", "/home/gurgui/after_sed.txt",
          {{"GPKI_BASEDIR", "WISKONSIN"}})
*/
static int __sed(strview src, strview dst,
  std::unordered_map<strview, strview> &&vals) {
  PDEBUG(2, "__sed()");

  std::ifstream srcfile(src.data());
  if (!srcfile.is_open()) {
    return GPKIH_FAIL;
  }
  std::ofstream dstfile(dst.data());
  if (!dstfile.is_open()) {
    return GPKIH_FAIL;
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
  return GPKIH_OK;
}                                                  

template <typename T> static int __is_valid_path(T path) {
  PDEBUG(2, "__is_valid_path()");

  if (gpkih::utils::fs::is_absolute_path(path) == false) {
    return GPKIH_FAIL;
  };
  try {
    if (fs::exists(path)) {
      auto ans = PROMPT("File or directory already exists, remove?", "[y/n]", true);
      if (ans == "y" || ans == "yes") {
        if(fs::remove_all(path) == true){
          return GPKIH_OK;
        }
        seterror("couldn't remove '{}' recursively",path);
        return GPKIH_FAIL;
      }
      return GPKIH_OK;
    };
  } catch (std::exception ex) {
    seterror("permission denied in '{}'\n", path);
    return GPKIH_OK;
  }

  return 1;
}

using namespace gpkih;
int __create_pki_filestruct(Profile &profile){
  PDEBUG(2, "__create_pki_filestruct()");

  // Create directories
  for (const std::string &relative : RELATIVE_DIRECTORY_PATHS()) {
    std::string path = fmt::format("{}{}{}",profile.source, SLASH, relative);
  
    if (!fs::create_directories(path)) {
      PERROR("couldn't create directory '{}'", path);
      // Remove the profile source dir
      // fs::remove_all(profile.source);
      return GPKIH_FAIL;
    }
  }
  // Create files
  for (const std::pair<std::string, std::string> &p : RELATIVE_FILE_PATHS()) {
    // p.first -> path
    // p.second -> default file contents
    std::string path = fmt::format("{}{}{}",profile.source,SLASH,p.first);

    std::ofstream(path, std::ios::app).write(p.second.c_str(), p.second.size());
    if (!fs::exists(path)) {
      seterror("couldn't create file " + p.first);
      return GPKIH_FAIL;
    }
  }

  // Copy required config files to profile
  for (auto &filenames :
       {vpn_conf_filename, pki_conf_filename}) {
    std::string src = fmt::format("{}{}", CONF_DIRPATH, filenames);
    std::string dst = fmt::format("{}{}{}", profile.source, SLASH, filenames);
    fs::copy(src, dst);
    if (!fs::exists(dst)) {
      PERROR("couldn't copy '{}' to '{}'\n", src, dst);
      return GPKIH_FAIL;
    }
  }
  
  std::string gopenssl_sed_src = CONF_DIRPATH + openssl_conf_filename;
  std::string gopenssl_sed_dst = fmt::format("{}{}{}",profile.source,SLASH,openssl_conf_filename);

// [Windows] - change \ for / since openssl processes slashes as / in the openssl.conf file
#ifdef _WIN32
  utils::str::replace_if(profile.source,utils::str::length(profile.source),'\\','/');
#endif

// [Linux | Windows] - adapt gopenssl.cnf 
  if (__sed(gopenssl_sed_src, gopenssl_sed_dst, {{"GPKIH_BASEDIR", fmt::format("{}/pki",profile.source)}})) {
    PERROR("__sed failed()\n");
    return GPKIH_FAIL;
  }

// [Windows] - change '/' slashes back to '\'
#ifdef _WIN32
  utils::str::replace_if(profile.source,utils::str::length(profile.source),'/','\\');
#endif

  // Add profile to database
  if (db::profiles::add(profile)) {
    // error is set by db::profiles::add
    printlasterror();
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
};


static int __check_profile_info(strview &profile_name, strview &profile_source, Profile &buffer){
  // TODO - check path length < 254 (uint8_t = 255 | 254 + '\0' = 255);
  // remember macro `len`
  Profile &profile = buffer;
  //if (!profile_name.empty() &&
  //    db::profiles::exists(profile_name)) {
  //  PWARN("profile '{}' already exists\n", profile_name);
  //}

  /* 
    profile_name can't be empty
    profile with name `profile_name` can't already exist
    profile_name size is max 254 characters
  */
  if (profile_name.empty() || db::profiles::exists(profile_name) || profile_name.size() > 254) {
    std::string pname;
    for(;;){
      pname = PROMPT("Desired profile name");

      if (db::profiles::exists(pname)) {
        PWARN("profile '{}' already exists\n", pname);
        continue;
      } else if (pname.empty()) {
        PWARN("profile name can't be empty\n");
        continue;
      }else if(pname.size() > 254){
        PWARN("max allowed length: 254 characters\n");
        continue;
      }
      CALLOCATE(profile.name,reinterpret_cast<size_t*>(&profile.namelen),pname);
      break;
    }
  } else {
    CALLOCATE(profile.name,reinterpret_cast<size_t*>(&profile.namelen),profile_name);
  }

  if (profile_source.empty() || !__is_valid_path(profile_source) || profile_source.size() > 254) {
    std::string psource;
    do {
      psource = PROMPT("Profile source dir (absolute path)");
    } while (!__is_valid_path(psource) && profile_source.size() > 254);
    CALLOCATE(profile.source,reinterpret_cast<size_t*>(&profile.sourcelen),psource);
  } else {
    CALLOCATE(profile.source,reinterpret_cast<size_t*>(&profile.sourcelen),profile_source);
  }

  // Check that we have write permissions in such path
  if (!__has_write_perms(profile.source)) {
    seterror("no write permissions in '{}'", profile.source);
    return GPKIH_FAIL;
  };

  PDEBUG(1,"creating profile source dir {}", profile.source);

  // Create source dir and any intermediary one in the path
  return fs::create_directories(profile.source) ? GPKIH_OK : GPKIH_FAIL;
} // check_profile_info()

int actions::init(strview &profile_name, strview &profile_source) {
  Profile profile;
  profile.last_modification = profile.creation_date = std::chrono::system_clock::now();
  
  if(__check_profile_info(profile_name, profile_source, profile) != GPKIH_OK){
    return GPKIH_FAIL;
  };

  if(__create_pki_filestruct(profile) == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  // sync() profiles.data with db::profiles::existing_profiles to add the new one
  db::profiles::sync();
  
  // Extra questions
  if (Config::get("behaviour", "prompt") == "yes") {
    bool autoans = Config::get("behaviour", "autoanswer") == "no" ? false : true; 
    if (autoans) {
      auto ans = PROMPT("Create dhparam? " + fmt::format(fg(fmt::terminal_color::bright_green) |
                                                  EMPHASIS::underline,
                                              "(highly recommended)"),
             "[y/n]", true);
      if (ans == "y" || ans == "yes") {
        utils::openssl::create_dhparam(fmt::format("{}{}pki{}tls{}dhparam1024",profile.source, SLASH, SLASH, SLASH));
      }

      ans = PROMPT("Create CA?","[y/n]",true);
      if(ans == "y" || ans == "yes"){
        Entity ca;
      }

    }else{
      PINFO("generating dhparam of {} bits\n", 1024);
      // QUESTION 1
      utils::openssl::create_dhparam(fmt::format("{}{}pki{}tls{}dhparam1024",profile.source, SLASH, SLASH, SLASH));
      
      // QUESTION 2

    }
  
  }

  ADD_LOG(L_INFO,"created profile [name:{},source:{}]",profile.name,profile.source);
  PSUCCESS("profile '{}' created\n", profile.name);
  return GPKIH_OK;
}