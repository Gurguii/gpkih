#include "actions.hpp"
static inline str openssl_conf_filename = "gopenssl.conf";

static inline std::vector<str> RELATIVE_DIRECTORY_PATHS(){
  return 
  {                                                           
    "tls", "pki" + SLASH + "ca", "pki" + SLASH + "keys",                       
    "pki" + SLASH + "crl", "pki" + SLASH + "serial", "packs",              
    "pki" + SLASH + "certs", "pki" + SLASH + "reqs",                       
    "pki" + SLASH + "database", "logs"                                     
  };
};                                               
static inline std::unordered_map<str,str> RELATIVE_FILE_PATHS(){
  return 
  {                                               
    {"pki" + SLASH + "crl" + SLASH + "crlnumber", "1000"},                     
    {"pki" + SLASH + "serial" + SLASH + "serial", "01"},                   
    {"pki" + SLASH + "database" + SLASH + "index.txt", ""},                
  };
}                                                    

int create_dhparam(strview outpath) {
  str command = fmt::format("openssl dhparam -out {} 2048", outpath);
  if (system(command.c_str())) {
    return -1;
  }
  return 0;
}
int create_openvpn_static_key(std::string_view outpath) {
  str command = fmt::format("openvpn --genkey tls-crypt {}", outpath);
  if (system(command.c_str())) {
    return -1;
  }
  return 0;
}

template <typename T> int IS_ABSOLUT_PATH(T path) {
#ifdef _WIN32
  return std::isalpha(path[0]);
#else
  return (path[0] == '/');
#endif
};
template <typename T> int IS_VALID_PATH(T path) {
  if (!IS_ABSOLUT_PATH(path)) {
    return 0;
  };
  try {
    if (fs::exists(path)) {
      str ans;
      PROMPT("file or directory already exists, remove?", "[y/n]");
      getline(std::cin, ans);
      if (ans == "y" || ans == "Y") {
        return fs::remove_all(
            path); // true if file got deleted - valid path (its free)
      }
      return 0;
    };
  } catch (std::exception ex) {
    PERROR("permission denied in '{}'\n", path);
    return 0;
  }

  return 1;
}

using namespace gpkih;
int actions::init(strview profile_name, strview profile_source) {
  Profile profile;
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

  // Create directories
  for (const str &relative : RELATIVE_DIRECTORY_PATHS()) {
    str path = profile.source + SLASH + relative;
    if (!fs::create_directories(path)) {
      PERROR("couldn't create directory '{}'", path);
      // Remove the profile source dir
      fs::remove_all(profile.source);
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
       {gpkih_conf_filename, vpn_conf_filename, pki_conf_filename}) {
    str src = fmt::format("{}{}", CONF_DIRPATH, filenames);
    str dst = fmt::format("{}{}{}", profile.source, SLASH, filenames);
    fs::copy(src, dst);
    if (!fs::exists(dst)) {
      PERROR("couldn't copy '{}' to '{}'\n", src, dst);
      return -1;
    }
  }

  // Adapt gopenssl.cnf file to the profile
  str sed_src = CONF_DIRPATH + openssl_conf_filename;
  str sed_dst = profile.source + SLASH + openssl_conf_filename;

#ifdef __WIN32
  std::replace_if(
      profile.source.begin(), profile.source.end(),
      [](char c) { return c == '\\'; }, '/');
#endif
  if (sed(sed_src, sed_dst, {{"GPKI_BASEDIR", profile.source + "/pki"}})) {
    PERROR("gsed failed()\n");
    return -1;
  }
#ifdef __WIN32
  std::replace_if(
      profile.source.begin(), profile.source.end(),
      [](char c) { return c == '/'; }, '\\');
#endif

  // Add profile to database
  if (db::profiles::add(&profile)) {
    return -1;
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
  return 0;
}
