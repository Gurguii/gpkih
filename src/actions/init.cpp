#include "actions.hpp"
#include <fmt/color.h>
#include <iostream> // std::cin
#include <fstream>
#include <sstream>

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
    PERROR(ex.what());
    return false;
  }
}

/* __sed("/home/gurgui/base", "/home/gurgui/after_sed.txt",
          {{"GPKI_BASEDIR", "WISKONSIN"}})
*/
static int __sed(std::string_view src, std::string_view dst,
  std::unordered_map<std::string_view, std::string_view> &&vals) {
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
  PDEBUG(2, "__is_valid_path()", path);

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
        PERROR("couldn't remove '{}' recursively",path);
        return GPKIH_FAIL;
      }
      return GPKIH_OK;
    };
  } catch (std::exception ex) {
    PERROR("permission denied in '{}'\n", path);
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
      PERROR("couldn't create file " + p.first);
      return GPKIH_FAIL;
    }
  }

  // Copy required config files to profile
  for (const auto &filename :
       {vpnConfFilename, pkiConfFilename}) {
    std::string src = fmt::format("{}{}", CONF_DIRPATH, filename);
    std::string dst = fmt::format("{}{}{}", profile.source, SLASH, filename);
    fs::copy(src, dst);
    if (!fs::exists(dst)) {
      PERROR("Couldn't copy '{}' to '{}'\n", src, dst);
      return GPKIH_FAIL;
    }
  }
  
  std::string gopenssl_sed_src = fmt::format("{}{}",CONF_DIRPATH,opensslConfFilename);
  std::string gopenssl_sed_dst = fmt::format("{}{}{}",profile.source,SLASH,opensslConfFilename);

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
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
};

static int __check_profile_info(std::string_view &profileName, std::string_view &profileSource, Profile &profile){
  // TODO - check path length < 254 (uint8_t = 255 | 254 + '\0' = 255);
  // remember macro `len`

  /* 
    profileName can't be empty
    profile with name `profileName` can't already exist
    profileName size is max 254 characters
  */
  if (profileName.empty() || db::profiles::exists(profileName) || profileName.size() > 254) {
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
    CALLOCATE(profile.name,reinterpret_cast<size_t*>(&profile.namelen),profileName);
  }

  if (profileSource.empty() || !__is_valid_path(profileSource) || profileSource.size() > 254) {
    std::string psource;
    do {
      psource = PROMPT("Profile source dir (absolute path)");
    } while (!__is_valid_path(psource) && profileSource.size() > 254);
    CALLOCATE(profile.source,reinterpret_cast<size_t*>(&profile.sourcelen),psource);
  } else {
    CALLOCATE(profile.source,reinterpret_cast<size_t*>(&profile.sourcelen),profileSource);
  }

  // Check that we have write permissions in such path
  if (!__has_write_perms(profile.source)) {
    PERROR("no write permissions in '{}'", profile.source);
    return GPKIH_FAIL;
  };

  PDEBUG(1,"creating profile source dir {}", profile.source);

  // Create source dir and any intermediary one in the path
  return fs::create_directories(profile.source) ? GPKIH_OK : GPKIH_FAIL;
} // check_profile_info()

int actions::init(std::string_view &profileName, std::string_view &profileSource) {
  Profile profile;
  profile.last_modification = profile.creation_date = std::chrono::system_clock::now();
  
  if(__check_profile_info(profileName, profileSource, profile) == GPKIH_FAIL){
    return GPKIH_FAIL;
  };

  if(__create_pki_filestruct(profile) == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  // Create profile entities file
  EntityManager eman{profileName};
  
  // sync() profiles.data with db::profiles::existing_profiles to add the new one
  db::profiles::sync();
  
  // Extra questions
  if (Config::get("behaviour", "prompt") == "yes") {
    bool autoans = Config::get("behaviour", "autoanswer") == "no" ? false : true;
    ProfileConfig pconf(profile, CONFIG_PKI);
    
    auto pkiconf = pconf.get(CONFIG_PKI);
    
    std::string_view keySize = pkiconf["key"]["size"];
    std::string_view keyAlgo = pkiconf["key"]["algorithm"];
    std::string_view days = pkiconf["crt"]["days"];

    if (autoans) {
      // QUESTION 1 - create dhparam?
      utils::openssl::create_dhparam(fmt::format("{}{}pki{}tls{}dhparam1024",profile.source, SLASH, SLASH, SLASH));
      // QUESTION 2 - create ca?
      ProfileConfig pconf(profile, CONFIG_PKI);
      Entity ca;
      ca.type = ET_CA;
      utils::entities::promptForSubject(profile.name, ca.subject, pconf, eman);

      build_ca(profile, pconf, ca, eman, days, keyAlgo, keySize);
    }else{
      auto ans = PROMPT("Create dhparam? " + fmt::format(fg(fmt::terminal_color::bright_green) |
                                                  EMPHASIS::underline,
                                              "(highly recommended)"),
             "[y/n]", true);
      if (ans == "y" || ans == "yes") {
        utils::openssl::create_dhparam(fmt::format("{}{}pki{}tls{}dhparam1024",profile.source, SLASH, SLASH, SLASH));
      }

      ans = PROMPT("Create CA?","[y/n]",true);
      if(ans == "y" || ans == "yes"){
        ProfileConfig pconf(profile, CONFIG_PKI);
        Entity ca;
        ca.type = ET_CA;
        utils::entities::promptForSubject(profile.name, ca.subject, pconf, eman);
  
        build_ca(profile, pconf, ca, eman, days, keyAlgo, keySize);
      }
    }
  }

  ADD_LOG(L_INFO,"profile:{} action:init source:{}",profile.name,profile.source);
  PSUCCESS("Profile '{}' created\n", profile.name);
  return GPKIH_OK;
}