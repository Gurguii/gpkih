#include "../db/database.hpp"
#include "../help/help.hpp"
#include "../modes/init.cpp"
#include "subparser.cpp"
#include <cstring>
#include <unordered_map>
#include <vector>
std::unordered_map<std::string, int (*)(std::vector<std::string>)> actions{
    {
        "build",
        gpki::subparser::build,
    },
    {"profile", gpki::subparser::profile},
    {"entity", gpki::subparser::entity}};

namespace gpki {
// [!] does not receive program name in args
int parse(int argc, const char **args) {
  if (argc == 0) {
    help::generic::usage();
    return -1;
  }
  // e.g test
  std::string profile = args[0];
  if (profile == "start" || profile == "init") {
    return create_new_profile();
  }
  // check if profile exists
  db::profiles::initialize();
  if (!db::profiles::exists(profile)) {
    std::cout << "profile '" << profile << "' doesn't exist\n";
    return -1;
    // not found
  }
  if (argc == 1) {
    std::cout << "action required\n";
    return -1;
  }
  // e.g build
  std::string action = args[1];

  //  check if actions exists
  if (actions.find(action) == actions.end()) {
    // not found
    std::cout << "action not found\n";
    return -1;
  }
  // action exists, call subparser
  actions[action];
  return 0;
}
} // namespace gpki
