#include "db/entities.cpp"
#include "db/profiles.cpp"
#include "help/generic.cpp"
#include "parse/parser.cpp"
#include "modes/build.cpp"
#include "modes/entity.cpp"
#include "modes/profile.cpp"
int main(int argc, const char **args) {
  // Add checks for openssl - openvpn existence
  // in the system, set a cache unordered_map<Profile> with all available profiles in profiles.csv
  // maybe keep track of the status of the csv on each execution so that we can check if something changed ( maybe use a simple hash?)
  if (parse(argc - 1, args + 1)) {
    return -1;
  }
  return 0;
}
