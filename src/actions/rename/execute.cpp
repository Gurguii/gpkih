#include "ARename.hpp"

#include "../../libs/printing/printing.hpp"
#include "../../db/profiles.hpp"
#include "../../gpkih.hpp"

using namespace gpkih;

int ARename::exec(std::vector<std::string> &args) const
{
	/* BEG - Parse args */
	DEBUG(1,"ARename::exec()");

	if(args.size() < 2){
		PHINT("./gpkih rename <profile_name> <new_name> [subopts]\n");
		return GPKIH_OK;
	}

	std::string_view oldProfileName = args[0];
	std::string_view newProfileName = args[1];

	auto profiles = db::profiles::get();
	auto iter = profiles->find(oldProfileName);

	/* BEG - Profile name/source checks */
	if(iter == profiles->end()){
		PERROR("Profile '{}' doesn't exist\n", oldProfileName);
		return GPKIH_FAIL;
	}

	if(profiles->find(newProfileName) != profiles->end()){
		PERROR("Profile '{}' already exists\n", newProfileName);
		return GPKIH_FAIL;
	}
	/* END - Profile name/source checks */
	
	if(DRY_RUN == true){
		PINFO("Renaming '{}' to '{}'\n", oldProfileName, newProfileName);
		return GPKIH_OK;
	}
	
	/* BEG - memory management */
	Profile _copy = iter->second;
	
	_copy.meta.lastModification = std::chrono::system_clock::now();
	_copy.meta.sourceLen = iter->second.meta.sourceLen;
	
	CALLOCATE(_copy.name, reinterpret_cast<size_t*>(&_copy.meta.nameLen), newProfileName);
	CALLOCATE(_copy.source, reinterpret_cast<size_t*>(&_copy.meta.sourceLen), iter->second.source);

	const char *p1 = iter->second.name;
	const char *p2 = iter->second.source;

	profiles->erase(iter);

	FREE_MEMORY_BLOCK(const_cast<char*>(p1), oldProfileName.size());
	FREE_MEMORY_BLOCK(const_cast<char*>(p2), _copy.meta.sourceLen + 1);
	/* END - memory management */

	/* BEG - Change database filename */
	std::string oldpath = fmt::format("{}{}_entities.data",DB_DIRPATH,oldProfileName);
	std::string newpath = fmt::format("{}{}_entities.data",DB_DIRPATH,newProfileName);

	if(profiles->emplace(_copy.name, _copy).second == false){
		return GPKIH_FAIL;
	}

	fs::rename(oldpath, newpath);

	if(!fs::exists(newpath)){
		PERROR("Couldn't rename db file\n");
		return GPKIH_FAIL;
	}
	/* END - Change database filename */

	PSUCCESS("Renamed profile '{}' to '{}'\n", oldProfileName, newProfileName);
	ADD_LOG(LL_INFO, fmt::format("profile:{} action:rename oldname:{}",newProfileName,oldProfileName));
	
	return db::profiles::sync();
	/* END - Parse args */
}