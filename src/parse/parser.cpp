#include "../db/database.hpp"
#include "../help/help.hpp"
#include <cstring>
#include <unordered_map>
#include <vector>
#include "subparser_build.cpp"
#include "subparser_profile.cpp"
#include "subparser_entity.cpp"

std::unordered_map<std::string, int (*)(std::vector<std::string>)> contexts{
  {"build",gpki::subparsers::build},
  {"profile", gpki::subparsers::profile},
  {"entity", gpki::subparsers::entity}};

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

  if (argc == 1) {
    std::cout << "action required\n";
    return -1;
  }
  // valid contexts: build|profile|entity
  std::string context = args[0];
  //  check if contexts exists and call appropiate subparser
  if (contexts.find(context) == contexts.end()) {
    // not found
    std::cout << "action not found\n";
    return -1;
  }
  // action exists, call subparser with subopts
  return contexts[context](std::vector<std::string>(args+1,args+argc));
}
} // namespace gpki
