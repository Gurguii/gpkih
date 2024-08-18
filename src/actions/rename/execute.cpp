#include "ARename.hpp"

#include "../../libs/printing/printing.hpp"
#include "../../db/profiles.hpp"
#include "../../gpkih.hpp"

using namespace gpkih;

int ARename::exec()
{
	/* BEG - Parse args */
	DEBUG(1,"ARename::exec()");

	if(args.size() < 2){
		PHINT("./gpkih rename <profile_name> <new_name> [subopts]\n");
		return GPKIH_OK;
	}

	std::string_view profile_name = args[0];
	if(db::profiles::exists(profile_name) == false){
		PWARN("Profile '{}' doesn't exist\n", profile_name);
		return GPKIH_OK;
	}

	Profile *profile = db::profiles::get(profile_name);
	if(profile == nullptr){
		PERROR("couldn't retrieve profile ptr");
		return GPKIH_FAIL;
	}
	PWARN("XD -> {}\n", profile->source);
	std::string oldpath = fmt::format("{}{}_entities.data",DB_DIRPATH,profile->name);
	
	FREE_MEMORY_BLOCK(const_cast<char*>(profile->name), profile->meta.nameLen+1);
	CALLOCATE(profile->name, reinterpret_cast<size_t*>(&profile->meta.nameLen), args[1]);
	PWARN("NEW NAME -> {} NEW SOURCE -> {} NEW SIZE -> {}\n", profile->name, profile->source, profile->meta.nameLen);
	std::string newpath = fmt::format("{}{}_entities.data",DB_DIRPATH,profile->name);

	if(DRY_RUN == true){
		PINFO("Renaming '{}' to '{}'\n", oldpath, newpath);
		return GPKIH_OK;
	}

	fs::rename(oldpath,newpath);

	if(fs::exists(newpath) == false){
		PERROR("couldn't rename old file");
		return GPKIH_FAIL;
	}
	
	profile->meta.lastModification = std::chrono::system_clock::now();

	if(db::profiles::sync() == GPKIH_FAIL){
		return GPKIH_FAIL;
	}
	
	PSUCCESS("Renamed profile '{}' to '{}'\n", args[0], args[1]);
	ADD_LOG(LL_INFO, fmt::format("profile:{} action:rename oldname:{}",args[1],args[0]));
	
	return GPKIH_OK;
	/* END - Parse args */
}