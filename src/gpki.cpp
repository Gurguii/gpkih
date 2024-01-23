#include "structs.hpp"

#include "gpki.hpp"

/* Parser & subparsers */
#include "parse/parser.cpp"

/* Database (csv) manipulation */
#include "db/entities.cpp"
#include "db/profiles.cpp"

/* Available actions */
#include "actions/actions.cpp"

/* Help functions */
#include "help/help.cpp"

#include "utils/checkpermissions.cpp"
int main(int argc, const char **args) {
  // Add checks for openssl - openvpn existence
  // in the system, set a cache unordered_map<Profile> with all available
  // profiles in profiles.csv maybe keep track of the status of the csv on each
  // execution so that we can check if something changed ( maybe use a simple
  // hash?)
  if (!fs::exists(BASEDIR)) {
    if (!fs::create_directory(BASEDIR)) {
      std::cout << "couldn't create directory '" + BASEDIR + "'\n";
      return -1;
    };
    str configdir = CURRENT_PATH + SLASH + ".." + SLASH + "config";
    std::cout << "configdir: " << configdir << "\n";
    fs::copy(configdir, CONFDIR, fs::copy_options::recursive);
    if (!fs::exists(CONFDIR) || !fs::is_directory(CONFDIR)) {
      return -1;
    }
    if (!fs::create_directory(DBDIR)) {
      return -1;
    };
  }
  db::profiles::initialize();
  // Check if gpkih install dir exists
  if (parse(argc - 1, args + 1)) {
    return -1;
  }
  return 0;
}
