#include "../help/help.hpp"
#include "subparser.cpp"
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
// does not receive program name in args
int parse(int argc, const char **args) {
  if (argc == 0) {
    help::generic::usage();
    return -1;
  }
  // e.g test
  std::string profile = args[0];
  // e.g build
  std::string action = args[1];

  // check if profile exists
  if () {
    // not found
  }

  //  check if actions exists
  if (actions.find(action) == actions.end()) {
    // not found
  }
  return 0;
}
} // namespace gpki
