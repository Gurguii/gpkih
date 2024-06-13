#include "parser.hpp"

int gpkih::parsers::rename(std::vector<std::string> &opts){
	PDEBUG(1,"gpkih::parsers::rename()");

	if(opts.size() < 2){
		PHINT("./gpkih rename <profile_name> <new_name> [subopts]\n");
		return GPKIH_OK;
	}

	std::string_view profile_name = opts[0];
	if(db::profiles::exists(profile_name) == false){
		PWARN("Profile '{}' doesn't exist\n", profile_name);
		return GPKIH_OK;
	}

	Profile *profile = db::profiles::get(profile_name);
	if(profile == nullptr){
		PERROR("couldn't retrieve profile ptr");
		return GPKIH_FAIL;
	}
	
	std::string oldpath = fmt::format("{}{}_entities.data",DB_DIRPATH,profile->name);
	CALLOCATE(profile->name, reinterpret_cast<size_t*>(&profile->namelen), opts[1]);
	std::string newpath = fmt::format("{}{}_entities.data",DB_DIRPATH,profile->name);

	fs::rename(oldpath,newpath);

	if(fs::exists(newpath) == false){
		PERROR("couldn't rename old file");
		return GPKIH_FAIL;
	}
	
	profile->last_modification = std::chrono::system_clock::now();

	if(db::profiles::sync() == GPKIH_FAIL){
		return GPKIH_FAIL;
	}
	
	PSUCCESS("Renamed profile '{}' to '{}'\n", opts[0], opts[1]);
	ADD_LOG(L_INFO, fmt::format("profile:{} action:rename oldname:{}",opts[1],opts[0]));
	
	return GPKIH_OK;
}