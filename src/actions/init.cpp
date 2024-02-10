#include "../utils/gpkih_util_funcs.hpp"
#include "actions.hpp"
#include <exception>

#define RELATIVE_DIRECTORY_PATHS                                               \
  std::vector<str> {                                                   \
    "tls", "pki" + SLASH + "ca", "pki" + SLASH + "keys",          \
        "pki" + SLASH + "crl", "pki" + SLASH + "serial", "packs",              \
        "pki" + SLASH + "certs", "pki" + SLASH + "reqs",                       \
        "pki" + SLASH + "database", "logs"                                     \
  }
#define RELATIVE_FILE_PATHS                                                    \
  std::unordered_map<str, str> {                               \
    {"pki" + SLASH + "crl" + SLASH + "crlnumber", "1000"},                     \
        {"pki" + SLASH + "serial" + SLASH + "serial", "01"}, {                 \
      "pki" + SLASH + "database" + SLASH + "index.txt", ""                     \
    }                                                                          \
  }

int create_dhparam(strview outpath) {
  str command =
      "openssl dhparam -out " + str(outpath) + " 2048";
  if (system(command.c_str())) {
    return -1;
  }
  return 0;
}
int create_openvpn_static_key(std::string_view outpath) {
  str command = "openvpn --genkey tls-crypt " + str(outpath);
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
    PROMPT("file or directory already exists, remove?","[y/n]");
    getline(std::cin, ans);
    if (ans == "y" || ans == "Y") {
      return fs::remove_all(
          path); // true if file got deleted - valid path (its free)
    }
    return 0;
  };
  }catch (std::exception ex) {
    PERROR("permission denied in '{}'\n",path);
    return 0;
  }

  return 1;
}

using namespace gpki;
int actions::init(subopts::init &params) {
  Profile profile;
  if (params.profile_name.empty() ||
      db::profiles::exists(params.profile_name)) {
    do {
      PROMPT("Desired profile name: ");
      std::getline(std::cin, profile.name);
      if(db::profiles::exists(profile.name)){
        PINFO("profile '{}' already exists\n");
        continue;
      }else if(profile.name.empty()){
        PINFO("profile name can't be empty\n");
        continue;
      }
      break;
    } while (db::profiles::exists(profile.name) || profile.name.empty());
  } else {
    profile.name = std::move(params.profile_name);
  }

  if (params.profile_source.empty() ||
      !IS_VALID_PATH(params.profile_source)) {
    do {
      PROMPT("Profile source dir (absolute path): ");
      std::getline(std::cin, profile.source);
    } while (!IS_VALID_PATH(profile.source));
  } else {
    profile.source = std::move(params.profile_source);
  }

  // Check that we have write permissions in such path
  if (!hasWritePermissions(profile.source)) {
    PERROR("no write permissions in '{}'",profile.source);
    return -1;
  };

  // Create directories
  for (const str &relative : RELATIVE_DIRECTORY_PATHS) {
    str path = profile.source + SLASH + relative;
    // std::cout << "directory: " << path << "\n";
    if (!fs::create_directories(path)) {
      PERROR("couldn't create directory '{}'",path);
      // Remove the profile source dir
      fs::remove_all(profile.source);
      return -1;
    }
  }
  // Create files
  for (const std::pair<str, str> &p : RELATIVE_FILE_PATHS) {
    // p.first -> path
    // p.second -> default file contents
    str path = profile.source + SLASH + p.first;
    std::ofstream(path, std::ios::app).write(p.second.c_str(), p.second.size());
    if (!fs::exists(path)) {
      seterror("couldn't create file " + p.first);
      return -1;
    }
  }
  // Copy templates.conf to profile
  str templates_src = CONFDIR + template_filename;
  str templates_dst = profile.source + SLASH + template_filename;
  fs::copy(templates_src,templates_dst);

  if(!fs::exists(templates_dst)){
    PERROR("couldn't copy '{}' to '{}'\n",templates_dst);
    return -1;
  }

  // Adapt gopenssl.cnf file to the profile
  str sed_src = CONFDIR + "gopenssl.cnf";
  str sed_dst = profile.source + SLASH + "gopenssl.cnf";

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
  
  // Extra questions
  if (prompt) {
    // QUESTION 1
    PROMPT("Create dhparam and openvpn tls key? (recommended)","[y/n]");
    str ans;
    getline(std::cin, ans);
    if (ans == "y" || ans == "Y") {
      create_openvpn_static_key(profile.source + SLASH + "tls" + SLASH +
                                "ta.key");
      create_dhparam(profile.source + SLASH + "tls" + SLASH + "dhparam2048");
    }
    ans.assign("");
    // QUESTION 2
    PROMPT("Create the CA now?","[y/n]");
    getline(std::cin, ans);
    if (ans == "y" || ans == "Y") {
      subopts::build default_params;
      default_params.type = ENTITY_TYPE::ca;
      default_params.profile = std::move(profile);
      if (actions::build(default_params)) {
        return -1;
      } else {
        PINFO("CA succesfully created\n");
        return -1;
      };
    }
  }
  return 0;
}