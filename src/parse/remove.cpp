#include "parser.hpp"
#include <sstream>
using namespace gpkih;

int parsers::remove(std::vector<std::string> opts) {
  /* Parse subopts */
  if (opts.empty()) {
    PERROR("profile name or '-all' must be given\n");
    PHINT("try gpki help remove\n");
    return -1;
  }
  auto iter = std::find(opts.begin(), opts.end(), "-all");
  std::vector<str> profiles_to_remove{};
  if (iter != opts.end()) {
    // There is a -all param in the arguments
    return actions::remove(profiles_to_remove,1);
  }
  
  sstream profiles(opts[0]);
  str profile;
  
  while(getline(profiles,profile,',')){
    if(db::profiles::exists(profile)){
      profiles_to_remove.emplace_back(profile);
    }else{
      PWARN("Profile '{}' doesn't exist, omitting\n", profile);
    }
  }
  
  return actions::remove(profiles_to_remove,0);
}