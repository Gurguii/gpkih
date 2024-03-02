#include "actions.hpp"
#include <iostream> // std::cin
#include <fstream>
#include <sstream>
static inline str openssl_conf_filename = "gopenssl.conf";

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

static inline std::vector<str> RELATIVE_DIRECTORY_PATHS(){
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
  };
};                                               
static inline std::unordered_map<str,str> RELATIVE_FILE_PATHS(){
  return 
  {  
    {fmt::format("pki{}crl{}crlnumber", SLASH, SLASH), "1000"},
    {fmt::format("pki{}serial{}serial", SLASH, SLASH), "01"},
    {fmt::format("pki{}database{}index.txt", SLASH, SLASH), ""}                                                         
  };
}                                                    

static int create_dhparam(strview outpath) {
  str command = fmt::format("openssl dhparam -out {} 2048", outpath);
  if (system(command.c_str())) {
    return -1;
  }
  return 0;
}

template <typename T> static int IS_ABSOLUT_PATH(T path) {
#ifdef _WIN32
  return std::isalpha(path[0]);
#else
  return (path[0] == '/');
#endif
};
template <typename T> static int IS_VALID_PATH(T path) {
  if (!IS_ABSOLUT_PATH(path)) {
    return 0;
  };
  try {
    if (fs::exists(path)) {
      str ans;
      PROMPT("File or directory already exists, remove?", "[y/n]");
      getline(std::cin, ans);
      if (ans == "y" || ans == "Y") {
        return (fs::remove_all(path) ? GPKIH_OK : GPKIH_FAIL); // true if file got deleted - valid path (its free)
      }
      return 0;
    };
  } catch (std::exception ex) {
    seterror("permission denied in '{}'\n", path);
    return 0;
  }

  return 1;
}

using namespace gpkih;
static int check_profile_info(strview profile_name, strview profile_source, Profile &buffer){
  Profile &profile = buffer;
  if (!profile_name.empty() &&
      db::profiles::exists(profile_name)) {
    PWARN("profile '{}' already exists\n", profile_name);
  }
  if (profile_name.empty() ||
      db::profiles::exists(profile_name)) {
    do {
      PROMPT("Desired profile name: ");
      std::getline(std::cin, profile.name);
      if (db::profiles::exists(profile.name)) {
        PWARN("profile '{}' already exists\n", profile.name);
        continue;
      } else if (profile.name.empty()) {
        PWARN("profile name can't be empty\n");
        continue;
      }
      break;
    } while (db::profiles::exists(profile.name) || profile.name.empty());
  } else {
    profile.name = std::move(profile_name);
  }

  if (profile_source.empty() || !IS_VALID_PATH(profile_source)) {
    do {
      PROMPT("Profile source dir (absolute path): ");
      std::getline(std::cin, profile.source);
    } while (!IS_VALID_PATH(profile.source));
  } else {
    profile.source = std::move(profile_source);
  }

  // Check that we have write permissions in such path
  if (!hasWritePermissions(profile.source)) {
    PERROR("no write permissions in '{}'", profile.source);
    return -1;
  };
  return GPKIH_OK;
}


int actions::init(strview profile_name, strview profile_source) {
  Profile profile;

  if(check_profile_info(profile_name, profile_source, profile) != GPKIH_OK){
    return -1;
  };

  // Create directories
  for (const str &relative : RELATIVE_DIRECTORY_PATHS()) {
    str path = profile.source + SLASH + relative;
    if (!fs::create_directories(path)) {
      PERROR("couldn't create directory '{}'", path);
      // Remove the profile source dir
      // fs::remove_all(profile.source);
      return -1;
    }
  }
  // Create files
  for (const std::pair<str, str> &p : RELATIVE_FILE_PATHS()) {
    // p.first -> path
    // p.second -> default file contents
    str path = profile.source + SLASH + p.first;
    std::ofstream(path, std::ios::app).write(p.second.c_str(), p.second.size());
    if (!fs::exists(path)) {
      seterror("couldn't create file " + p.first);
      return -1;
    }
  }
  // Copy required config files to profile
  for (auto &filenames :
       {vpn_conf_filename, pki_conf_filename}) {
    str src = fmt::format("{}{}", CONF_DIRPATH, filenames);
    str dst = fmt::format("{}{}{}", profile.source, SLASH, filenames);
    fs::copy(src, dst);
    if (!fs::exists(dst)) {
      PERROR("couldn't copy '{}' to '{}'\n", src, dst);
      return -1;
    }
  }
  
  str gopenssl_sed_src = CONF_DIRPATH + openssl_conf_filename;
  str gopenssl_sed_dst = profile.source + SLASH + openssl_conf_filename;

// [Windows] - change \ for / since openssl processes slashes as / in the openssl.conf file
#ifdef _WIN32
  std::replace_if(
      profile.source.begin(), profile.source.end(),
      [](char c) { return c == '\\'; }, '/');
#endif
  // adapt gopenssl.cnf 
  if (sed(gopenssl_sed_src, gopenssl_sed_dst, {{"GPKI_BASEDIR", profile.source + "/pki"}})) {
    PERROR("gsed failed()\n");
    return -1;
  }
// [Windows] - change '/' slashes back to '\'
#ifdef _WIN32
  std::replace_if(
      profile.source.begin(), profile.source.end(),
      [](char c) { return c == '/'; }, '\\');
#endif

  // Add profile to database
  if (db::profiles::add(&profile)) {
    // error is set by db::profiles::add
    printlasterror();
    return GPKIH_FAIL;
  }
  // Initialize entities db
  db::entities::initialize(profile.name);
  if(!db::entities::initialized){
    return GPKIH_FAIL;
  }
  // Extra questions
  if (Config::get("behaviour", "prompt") == "yes") {
    // QUESTION 1
    if (Config::get("behaviour", "autoanswer") == "no") {
      PROMPT("Create dhparam? " + fmt::format(fg(COLOR::lime) |
                                                  EMPHASIS::underline,
                                              "(highly recommended)"),
             "[y/n]");
      str ans;
      getline(std::cin, ans);
      if (ans == "y" || ans == "Y") {
        create_dhparam(profile.source + SLASH + "tls" + SLASH + "dhparam2048");
      }
    }else{
      create_dhparam(profile.source + SLASH + "tls" + SLASH + "dhparam2048");
    }
  }
  return GPKIH_OK;
}
