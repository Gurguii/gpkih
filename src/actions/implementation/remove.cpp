#include "../actions.hpp"
#include "../../db/profiles.hpp"

using namespace gpkih;

int actions::remove(std::vector<std::string> &profiles_to_remove, int remove_all) {
  if (remove_all) {
    size_t deletedFiles = 0;
    size_t profileCount = db::profiles::size();

    if(db::profiles::remove_all(&deletedFiles) == GPKIH_FAIL){
      return GPKIH_FAIL;
    };

    PSUCCESS("Deleted [{}] profiles [{}] files\n", profileCount, deletedFiles); 
    
    return GPKIH_OK;
  }

  std::vector<std::pair<std::string_view,std::string_view>> removed {};

  for (auto p : profiles_to_remove) {
    const Profile* profile = db::profiles::get(p);
    if(profile == nullptr){
      PWARN("Profile '{}' doesn't exist\n", p);    
    }else{
      if (fs::remove_all(profile->source) == 0){
        PWARN("Couldn't remove source dir '{}'\n", profile->source);
      }
      if (db::profiles::remove(p) == GPKIH_FAIL){
        PWARN("Deleted source dir but something failed syncing db\n");
      }
      removed.push_back(std::make_pair<std::string_view,std::string_view>(profile->name,profile->source));
    }
  }

  db::profiles::sync();

  /* Build the success message */
  size_t amount = removed.size();

  std::ostringstream removedProfileNames{};
  
  for(const auto &[name,source] : removed){
    removedProfileNames << name << ",";
  
  }
  
  std::string removedProfileNamesStr{removedProfileNames.str()};
  PSUCCESS("Removed '{}' profiles: {}\n", amount, removedProfileNamesStr.erase(removedProfileNamesStr.size()-1));
  
  std::ostringstream logMessage{};

  for(const auto &[name,source] : removed){
    logMessage << "{" << name << ":" << source << "}" << ",";
  }

  std::string logMessageStr{logMessage.str()};
  ADD_LOG(LL_INFO,fmt::format("action:remove profiles:{}",logMessageStr.erase(logMessageStr.size()-1)));
 
  return removed.size();
}