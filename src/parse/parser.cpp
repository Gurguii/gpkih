#include <cstring>
#include <unordered_map>
#include <vector>

#include "../db/database.hpp"
#include "../help/help.hpp"
#include "../gpki.hpp"

#include "subparser_build.cpp"
#include "subparser_gencrl.cpp"
#include "subparser_init.cpp"
#include "subparser_list.cpp"
#include "subparser_revoke.cpp"
#include "subparser_remove.cpp"
#include "subparser_remove_all.cpp"

std::unordered_map<str, int (*)(Profile *, std::vector<str>)>
    valid_actions_subparsers{
        {"build", gpki::subparsers::build},
        {"revoke", gpki::subparsers::revoke},
        {"gencrl", gpki::subparsers::gencrl},
        {"remove",gpki::subparsers::remove},
    };

namespace gpki {
// [!] parse() does not expect to receive program name in args
int parse(int argc, const char **_args) {
  if (argc == 0) {
    help::usage();
    return -1;
  }
  std::vector<str> args(_args,_args+argc);
  int s = args.size();
  for(int i = 0; i < s; ++i){
    strview op = args[i];
    if (op == "-y" || op == "--noprompt"){
      prompt = 0;
      args.erase(args.begin()+i);
      --s;
    }
  }
  if(args.size() == 0){
    help::usage();
    return -1;
  }
  str action = args[0];
  /* ACTIONS THAT DO NOT REQUIRE A PROFILE */
  if (action == "help") {
    if (args.size() > 1) {
      call_helper(args[1]);
    } else {
      help::usage();
    }
    return 0;
  } else if (action == "init") {
    args.erase(args.begin());
    return subparsers::init(std::move(args));
  } else if (action == "list") {
    args.erase(args.begin());
    return subparsers::list(std::move(args));
  }else if(action == "remove-all"){
    return subparsers::remove_all();
  }
  // check if action exists and call appropiate subparser
  if (valid_actions_subparsers.find(action) == valid_actions_subparsers.end()) {
    // not found
    PERROR("action '{}' not found\n", action);
    return -1;
  }
  if (argc == 1) {
    call_helper(action);
    return -1;
  }
  str profile_name = args[1];
  Profile current_profile;
  if (db::profiles::load(profile_name, current_profile)) {
    // profile doesn't exist
    PINFO("profile '{}' doesn't exist\n", profile_name);
    call_helper(action);
    return -1;
  };
  db::entities::initialize(profile_name);
  // action exists, call subparser with subopts
  args.erase(args.begin());
  return valid_actions_subparsers[action](
      &current_profile, std::move(args));
}
} // namespace gpki
