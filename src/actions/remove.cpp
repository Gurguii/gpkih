#include "actions.hpp"
#include <filesystem>
using namespace gpkih;

int actions::remove(std::vector<str> &profiles_to_remove, int remove_all) {
  if (remove_all) {
    size_t ndeleted = db::profiles::remove_all();
    PSUCCESS("Deleted [{}] profiles\n", ndeleted); 
    return ndeleted >= 0 ? GPKIH_OK : GPKIH_FAIL;
  }

  std::vector<std::string> removed {};

  for (auto p : profiles_to_remove) {
    const Profile* profile = db::profiles::get(p);
    if(profile == nullptr){
      PWARN("Profile '{}' doesn't exist\n", p);    
    }else{
      if (fs::remove_all(profile->source) == -1){
        PWARN("Couldn't remove source dir '{}'\n", profile->source);
      }
      if (db::profiles::remove(p) == GPKIH_FAIL){
        PWARN("Deleted source dir but something failed syncing db\n");
      }
      removed.push_back(p);
    }
  }

  db::profiles::sync();

  /* Build the success message */
  size_t amount = removed.size();

  std::stringstream removedProfileNames{};
  for(const auto &st : removed){
    removedProfileNames << st << ",";
  }

  /* Remove trailing comma */
  std::string s{removedProfileNames.str()};
  s.erase(s.end()-1, s.end());

  PSUCCESS("Removed profiles: {}\n", s);
  ADD_LOG(L_INFO, "action:remove removed_profiles:{}",s);
 
  return removed.size();
}