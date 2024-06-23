#include "../parser.hpp"

using namespace gpkih;

int parsers::init(std::vector<std::string> &opts) {
  opts.push_back("\0");

  std::string_view profileName, profileSource;
  for (int i = 0; i < opts.size() - 1; ++i) {
    std::string_view opt = opts[i];
    if (opt == "-n" || opt == "--name") {
      profileName = opts[++i];
    } else if (opt == "-s" || opt == "--source") {
      profileSource = opts[++i];
    } else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }
  
  actions::init(profileName,profileSource);
  return 0;
}
