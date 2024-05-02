#include "parser.hpp"

using namespace gpkih;

int parsers::init(std::vector<std::string> &opts) {
  opts.push_back("\0");
  strview profile_name, profile_source;
  for (int i = 0; i < opts.size() - 1; ++i) {
    std::string_view opt = opts[i];
    if (opt == "-n" || opt == "--name") {
      profile_name = opts[++i];
    } else if (opt == "-s" || opt == "--source") {
      profile_source = opts[++i];
    } else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }
  actions::init(profile_name,profile_source);
  return 0;
}
