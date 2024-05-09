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
		seterror("couldn't retrieve profile ptr");
		return GPKIH_FAIL;
	}
	
	std::string oldpath = fmt::format("{}{}_entities.data",DB_DIRPATH,profile->name);
	CALLOCATE(profile->name, reinterpret_cast<size_t*>(&profile->namelen), opts[1]);
	std::string newpath = fmt::format("{}{}_entities.data",DB_DIRPATH,profile->name);

	fs::rename(oldpath,newpath);

	if(fs::exists(newpath)){
		profile->last_modification = std::chrono::system_clock::now();
		db::profiles::sync();
		PSUCCESS("renamed profile '{}' to '{}'\n", opts[0], opts[1]);
		ADD_LOG(L_INFO, "renamed profile '{}' to '{}'", opts[0], opts[1]);
		return GPKIH_OK;
	}else{
		seterror("couldn't rename old file");
		return GPKIH_FAIL;
	}
}