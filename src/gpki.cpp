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

int main(int argc, const char **args) {
  // Add checks for openssl - openvpn existence
  // in the system, set a cache unordered_map<Profile> with all available profiles in profiles.csv
  // maybe keep track of the status of the csv on each execution so that we can check if something changed ( maybe use a simple hash?)
  for(int i = 0; i < argc; ++i){
    globals::user_used_command += std::string(args[i]) + " ";
  }
  db::initialize_dbs();
  if (parse(argc - 1, args + 1)) {
    return -1;
  }
  return 0;
}
