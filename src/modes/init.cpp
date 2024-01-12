#include "../db/database.hpp"
#include "../gpki.hpp"
#include <iostream>
#include <unordered_map>
#include <vector>
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
  std::string command = "openssl dhparam -out " + std::string(outpath) + "1024";
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
using namespace gpki;
int create_new_profile() {
  {
    do {
      std::cout << "[+] Please introduce desired profile name: ";
      std::getline(std::cin, (*PROFILE).name);
    } while (db::profiles::exists(PROFILE));

    do {
      std::cout << "[+] Please introduce pki base dir (absolute path): ";
      std::getline(std::cin, (*PROFILE).source);
    } while (!IS_ABSOLUT_PATH((*PROFILE).source));

    // Check that we have write permissions in such path
    if (!hasWritePermissions((*PROFILE).source)) {
      seterror("[error] Not write permissions in '" + (*PROFILE).source +
               "'\n");
      return -1;
    };

    // Create directories
    for (const std::string &relative : RELATIVE_DIRECTORY_PATHS) {
      std::string path = (*PROFILE).source + SLASH + relative;
      if (!std::filesystem::create_directories(path)) {
        seterror("couldn't create directory " + path);
        // Remove the profile source dir
        std::filesystem::remove_all((*PROFILE).source);
      }
    }
    // Create files
    for (const std::pair<std::string, std::string> &p : RELATIVE_FILE_PATHS) {
      // p.first -> path
      // p.second -> default file contents
      std::string path = (*PROFILE).source + SLASH + p.first;
      std::ofstream(path, std::ios::app)
          .write(p.second.c_str(), p.second.size());
      if (!std::filesystem::exists(path)) {
        seterror("couldn't create file " + p.first);
        return -1;
      }
    }

    // Add profile to database
    if (db::profiles::add(PROFILE)) {
      return -1;
    }

    // Create extra tls keys
    if (globals::prompt) {
      std::cout << "Create dhparam and openvpn static key (tls-crypt)? Y/N: ";
      char c = getchar();
      if (c == 'y' || c == 'Y') {
        create_openvpn_static_key((*PROFILE).source + SLASH + "tls" + SLASH +
                                  "ta.key");
        create_dhparam((*PROFILE).source + SLASH + "tls" + SLASH +
                       "dhparam1024");
      }
    }
    return 0;
  }
}
