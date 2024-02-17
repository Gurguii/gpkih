#include "parser.hpp"

using namespace gpkih;

int parsers::remove(std::vector<std::string> opts) {
  /* Parse subopts */
  if (opts.empty()) {
    PERROR("profile name or '-all' must be given\n");
    PHINT("try gpki help remove\n");
    return -1;
  }
  auto iter = std::find(opts.begin(), opts.end(), "-all");
  subopts::remove params;
  if (iter != opts.end()) {
    // There is a -all param in the arguments
    params.all = 1;
    return actions::remove(params);
  }
  sstream names(opts[0]);
  str profile;
  while (getline(names, profile, CSV_DELIMITER_c)) {
    if (!db::profiles::exists(profile)) {
      PWARN("Profile '{}' doesn't exist, omitting\n", profile);
      continue;
    }
    params.profiles.emplace_back(profile);
  }
  return actions::remove(params);
}
