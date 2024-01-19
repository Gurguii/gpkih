#include <cstring>
#include <unordered_map>
#include <vector>

#include "../db/database.hpp"
#include "../help/help.hpp"

#include "subparser_init.cpp"
#include "subparser_build.cpp"
#include "subparser_revoke.cpp"

std::unordered_map<std::string, int (*)(std::vector<std::string>)> actions{
  {"init", gpki::subparsers::init},
  {"build",gpki::subparsers::build},
  {"revoke",gpki::subparsers::revoke},
};

namespace gpki {
// [!] parse() does not expect to receive program name in args
int parse(int argc, const char **args) {
  if (argc == 0) {
    help::generic::usage();
    return -1;
  }
  if (db::profiles::initialize()){
    std::cerr << "[error] couldn't initialize database\n";
    return -1;
  }
  // std::string first = args[1];
  // if (first == "init") {
  //   return actions["init"](std::vector<std::string>(args+1,args+argc));
  // }else if(argc == 1){
  //   std::cout << "action required\n";
  //   return -1;
  // }
  std::string action = args[0];
  //  check if actions exists and call appropiate subparser
  if (actions.find(action) == actions.end()) {
    // not found
    std::cout << "action not found\n";
    return -1;
  }
  // action exists, call subparser with subopts
  return actions[action](std::vector<std::string>(args+1,args+argc));
}
} // namespace gpki
