#include "actions.hpp"
#include <filesystem>
using namespace gpkih;

int actions::remove(std::vector<str> &profiles_to_remove, int remove_all) {
  if (remove_all) {
    size_t deleted = db::profiles::remove_all();
    PSUCCESS("Deleted [{}] profiles\n", deleted); 
    return deleted >= 0 ? GPKIH_OK : GPKIH_FAIL;
  }

  std::vector<std::string> removed {};

  for (auto p : profiles_to_remove) {
    auto profile = db::profiles::get(p);
    if(profile == nullptr){
      PWARN("profile '{}' doesn't exist\n", p);    
    }else{
      fs::remove_all(profile->source);
      db::profiles::remove(p);
      removed.push_back(p);
    }
  }

  db::profiles::sync();

  /* Build the success message */
  size_t amount = removed.size();
  std::stringstream ss{};
  ss << "Removed [" << amount << "] " << "profile/s: ";

  for(const auto &st : removed){
    ss << st << ",";
  }

  std::string s{ss.str()};
  s.erase(s.end()-1, s.end());

  PSUCCESS("{}\n", s);
  return removed.size();
}