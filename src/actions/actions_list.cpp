#include "actions.hpp"

using namespace gpki;
int actions::list(subopts::list &params) {
  if (params.profile.empty() && params.common_name.empty()) {
    // all profiles and all entities
    for (auto kv : db::profiles::existing_profiles) {
      // first: profile name | second: source
      std::cout << "================================\n";
      std::cout << "Profile name: " << kv.first << "\n";
      std::cout << "Profile source: " << kv.second.source << "\n";
      std::cout << "================================\n";
    }
  } else if (params.profile.empty() && !params.common_name.empty()) {
    // target entity/s and all profiles
  } else if (!params.profile.empty() && params.common_name.empty()) {
    // target profile/s and all entities
  } else {
    // target profile/s and entity/s
  }
  return 0;
}
