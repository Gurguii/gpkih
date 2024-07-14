#include "../parser.hpp"

using namespace gpkih;

int parsers::remove(std::vector<std::string> &opts) {
  DEBUG(1, "parsers::remove()");

  /* Parse subopts */
  if (opts.empty()) {
    PERROR("profile name or '-all|--all' must be given\n");
    PHINT("try gpki help remove\n");
    return -1;
  }

  std::vector<std::string> profiles_to_remove{};
  
  if (std::find(opts.begin(), opts.end(), "-all") != opts.end() || std::find(opts.begin(), opts.end(), "--all") != opts.end()) {
    // There is a -all param in the arguments
    size_t deleted_profiles = db::profiles::remove_all();
    PSUCCESS("deleted [{}] profiles\n", deleted_profiles);
    return GPKIH_OK;
  }
  
  std::stringstream profiles(opts[0]);
  std::string profile;
  
  while(getline(profiles,profile,',')){
    if(db::profiles::exists(profile)){
      profiles_to_remove.emplace_back(profile);
    }else{
      PWARN("Profile '{}' doesn't exist, omitting\n", profile);
    }
  }
  
  return actions::remove(profiles_to_remove,0);
}