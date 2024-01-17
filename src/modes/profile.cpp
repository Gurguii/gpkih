#include "profile.hpp"
#include "build.hpp"
#include <ostream>

#define RELATIVE_DIRECTORY_PATHS                                               \
  std::vector<std::string> {                                                   \
    "tls", "templates", "pki" + SLASH + "ca", "pki" + SLASH + "keys",          \
        "pki" + SLASH + "crl", "pki" + SLASH + "serial", "packs",              \
        "pki" + SLASH + "certs", "pki" + SLASH + "reqs",                       \
        "pki" + SLASH + "database", "logs"                                     \
  }
#define RELATIVE_FILE_PATHS                                                    \
  std::unordered_map<std::string, std::string> {                               \
    {"pki" + SLASH + "crl" + SLASH + "crlnumber", "1000"},                     \
        {"pki" + SLASH + "serial" + SLASH + "serial", "01"}, {                 \
      "pki" + SLASH + "database" + SLASH + "index.txt", ""                     \
    }                                                                          \
  }
#define seterror(x) std::cout << "[error] - " << x << "\n";
bool hasWritePermissions(const std::filesystem::path &directoryPath) {
  /* this approach is kind of sad to see */
  try {
    auto s = std::filesystem::status(directoryPath);
    return true;
  } catch (std::exception &ex) {
    return false;
  }
}
int create_dhparam(std::string_view outpath) {
  std::string command = "openssl dhparam -out " + std::string(outpath) + " 1024";
  if (system(command.c_str())) {
    return -1;
  }
  return 0;
}
int create_openvpn_static_key(std::string_view outpath) {
  std::string command = "openvpn --genkey tls-crypt " + std::string(outpath);
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
template <typename T> int IS_VALID_PATH(T path){
  if(!IS_ABSOLUT_PATH(path)){
    return 0;
  };
  if(std::filesystem::exists(path) ){
    std::string ans;
    std::cout << "file or directory '" << path << "' already exists, remove? y/n ";
    getline(std::cin,ans);
    if(ans == "y" || ans == "Y"){
      return std::filesystem::remove_all(path); // true if file got deleted - valid path (its free)
    }
    return 0;
  };
  return 1;
}

using namespace gpki;
int modes::profile::create() {
  Profile profile;
  do {
    std::cout << "[+] Please introduce desired profile name: ";
    std::getline(std::cin, profile.name);
  } while (db::profiles::exists(&profile));

  do {
    std::cout << "[+] Please introduce pki base dir (absolute path): ";
    std::getline(std::cin, profile.source);
  } while (!IS_VALID_PATH(profile.source));
  
  // Check that we have write permissions in such path
  if (!hasWritePermissions(profile.source)) {
    seterror("[error] Not write permissions in '" + profile.source +
             "'\n");
    return -1;
  };

  // Create directories
  for (const std::string &relative : RELATIVE_DIRECTORY_PATHS) {
    std::string path = profile.source + SLASH + relative;
    if (!std::filesystem::create_directories(path)) {
      seterror("couldn't create directory " + path);
      // Remove the profile source dir
      std::filesystem::remove_all(profile.source);
    }
  }
  // Create files
  for (const std::pair<std::string, std::string> &p : RELATIVE_FILE_PATHS) {
    // p.first -> path
    // p.second -> default file contents
    std::string path = profile.source + SLASH + p.first;
    std::ofstream(path, std::ios::app)
        .write(p.second.c_str(), p.second.size());
    if (!std::filesystem::exists(path)) {
      seterror("couldn't create file " + p.first);
      return -1;
    }
  }
  // Adapt gopenssl.cnf file to the profile
  std::string sed_src = globals::configdir + SLASH + "gopenssl.cnf";
  std::string sed_dst = profile.source + SLASH + "gopenssl.cnf";
  
  #ifdef __WIN32
  std::replace_if(profile.source.begin(),profile_source.end(),[](char c){return c == '\\';},'/');
  #endif
  if(sed(sed_src,sed_dst,{{"GPKI_BASEDIR",profile.source + "/pki"}})){
    std::cout << "gsed() failed\n";
    return -1;
  }
  #ifdef __WIN32
  std::replace_if(profile.source.begin(),profile_source.end(),[](char c){return c == '/';},'\\');
  #endif

  // Add profile to database
  if (db::profiles::add(&profile)) {
    return -1;
  }

  // Extra questions
  if (globals::prompt) {
    // QUESTION 1
    std::cout << "Create dhparam and openvpn static key (tls-crypt)? [recommended] Y/N: ";
    std::string ans;
    getline(std::cin,ans);
    if (ans == "y" || ans == "Y") {
      create_openvpn_static_key(profile.source + SLASH + "tls" + SLASH +
                                "ta.key");
      create_dhparam(profile.source + SLASH + "tls" + SLASH +
                     "dhparam1024");
    }
    ans.assign("");
    // QUESTION 2
    std::cout << "Do you want to create the CA now? Y/N: ";
    getline(std::cin,ans);
    if(ans == "y" || ans == "Y"){
      Entity ca;
      ca.type = "ca";
      ca.profile_name = profile.name;
      build::build_params default_params;
      build::get_entity(&profile,&ca,&default_params); 
      auto a = build::get_openssl_command(&profile,&ca,&default_params);
      if(!a.has_value()){
        return -1;
      }
      for(auto command : a.value()){
        if(system(command.c_str())){
          std::cout << "command '" << command << "' failed\n";
          return -1;
        }
      }
      db::entities::initialize();
      if(db::entities::add(&ca)){
        return -1;
      };
      std::cout << "CA '" << ca.subject.cn << "' succesfully created\n";
    }
  }
  return 0;
}
