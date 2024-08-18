#include "ARemove.hpp"

#include "../../libs/printing/printing.hpp"
#include "../../db/profiles.hpp"
#include "../../libs/logger/enums.hpp"
#include "../../gpkih.hpp"

#include <algorithm>
#include <sstream>

using namespace gpkih;

static int removeProfile(std::vector<std::string> &profiles_to_remove, int remove_all){
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
  return GPKIH_OK;
}

int ARemove::exec()
{
	/* BEG - Parse args */
	DEBUG(1, "parsers::remove()");

  	/* Parse subopts */
  	if (args.empty()) {
  	  PERROR("profile name or '-all|--all' must be given\n");
  	  PHINT("try gpki help remove\n");
  	  return -1;
  	}
	
  	std::vector<std::string> profiles_to_remove{};
  	
  	if (std::find(args.begin(), args.end(), "-all") != args.end() || std::find(args.begin(), args.end(), "--all") != args.end()) {
  	  // There is a -all param in the arguments
  	  size_t deleted_profiles = removeProfile(profiles_to_remove, 1);
  	  PSUCCESS("deleted [{}] profiles\n", deleted_profiles);
  	  return GPKIH_OK;
  	}
  	
  	std::stringstream profiles(args[0]);
  	std::string profile;
  	
  	while(getline(profiles,profile,',')){
  	  if(db::profiles::exists(profile)){
  	    profiles_to_remove.emplace_back(profile);
  	  }else{
  	    PWARN("Profile '{}' doesn't exist, omitting\n", profile);
  	  }
  	}
	/* END - Parse args */

  	return removeProfile(profiles_to_remove, 0);
}
