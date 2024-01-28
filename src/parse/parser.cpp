#include <cstring>
#include <unordered_map>
#include <vector>

#include "../db/database.hpp"
#include "../help/help.hpp"

#include "subparser_build.cpp"
#include "subparser_gencrl.cpp"
#include "subparser_init.cpp"
#include "subparser_list.cpp"
#include "subparser_revoke.cpp"

std::unordered_map<std::string, int (*)(Profile *, std::vector<std::string>)>
    valid_actions_subparsers{
        {"build", gpki::subparsers::build},
        {"revoke", gpki::subparsers::revoke},
        {"gencrl", gpki::subparsers::gencrl},
    };

namespace gpki {
// [!] parse() does not expect to receive program name in args
int parse(int argc, const char **args) {
  if (argc == 0) {
    help::usage();
    return -1;
  }

  std::string action = args[0];

  if (action == "help") {
    if (argc > 1) {
      call_helper(args[1]);
    } else {
      help::usage();
    }
    return 0;
  } else if (action == "init") {
    return subparsers::init(std::vector<std::string>(args + 1, args + argc));
  } else if (action == "list") {
    return subparsers::list(std::vector<std::string>(args + 1, args + argc));
  }

  // check if action exists and call appropiate subparser
  if (valid_actions_subparsers.find(action) == valid_actions_subparsers.end()) {
    // not found
    std::cout << "action '" << action << "'not found\n";
    return -1;
  }
  if (argc == 1) {
    call_helper(action);
    return -1;
  }
  std::string profile_name = args[1];
  Profile current_profile;
  if (db::profiles::load(profile_name, current_profile)) {
    // profile doesn't exist
    std::cout << "Profile '" << profile_name << "' doesn't exist\n";
    call_helper(action);
    return -1;
  };
  db::entities::initialize(profile_name);
  // action exists, call subparser with subopts
  return valid_actions_subparsers[action](
      &current_profile, std::vector<std::string>(args + 2, args + argc));
}
} // namespace gpki
