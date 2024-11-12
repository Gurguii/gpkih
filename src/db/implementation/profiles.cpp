#include "../profiles.hpp"
#include "../../gpkih.hpp"
#include "../../libs/printing/printing.hpp"
#include "../mnck.hpp"
#include <filesystem>

constexpr size_t kProfileSafeBufferSize = sizeof(ProfileMetadata) + GPKIH_MAX_PATH + GPKIH_MAX_VARCHAR + 2; // 4390
static std::map <std::string_view, Profile > existing_profiles{};

using namespace gpkih;
// Returns id from idfile and increases it, updating the file
static uint64_t nextID(){
	std::fstream file(db::profiles::idfile, std::ios::binary | std::ios::in | std::ios::out);
	
	if(!file.is_open()){
		PERROR("couldn't open id file '{}' for reading\n", db::profiles::idfile);
		return GPKIH_FAIL;
	}

	uint64_t id = 0;
	file.read(reinterpret_cast<char*>(&id), sizeof(decltype(id)));

	if(file.fail()){
		PERROR("couldn't read from id file\n");
		return GPKIH_FAIL;
	}

	uint64_t nid = id+1;
	file.seekp(SEEK_SET);
	file.write(reinterpret_cast<const char*>(&nid), sizeof(decltype(nid)));
	if(file.fail()){
		PERROR("couldn't write to id file\n");
		return GPKIH_FAIL;
	}

	file.close();

	return id;
}

std::map <std::string_view, Profile>* const db::profiles::get(){
	return &existing_profiles;
}

// overwrite dbpath (data.bin) with the profiles in existing_profiles
int db::profiles::sync(){
	DEBUG(1,"db::profiles::sync()");

	std::ofstream file(dbpath, std::ios::binary);
	if(!file.is_open()){
		PERROR("couldn't open db file for writing '{}' - db::profiles::sync()\n", dbpath);
		return GPKIH_FAIL;
	}

	size_t s = existing_profiles.size();

	if(mnck::dump(file,s) == false){
		return GPKIH_FAIL;
	};

	gurgui::memory::SmartMemBlock _buff(kProfileSafeBufferSize);
	auto buffer = _buff.get();

	if(buffer == nullptr){
		throw("Couldn't allocate buffer to synchronize profiles, calloc() returned nullptr");
	}

	int pos = 0;
	for(const auto &[name, profile] : existing_profiles){
		DEBUGF(3, "Writing profile [{}:{},{}:{}]", name,profile.meta.nameLen, profile.source, profile.meta.sourceLen);
		memcpy(buffer, &profile.meta, sizeof(ProfileMetadata));
		pos+=sizeof(ProfileMetadata);

		memcpy(buffer+pos, profile.name, profile.meta.nameLen);
		pos+=profile.meta.nameLen + 2;

		memcpy(buffer+pos, profile.source, profile.meta.sourceLen);
		pos+=profile.meta.sourceLen + 2;

		file.write(reinterpret_cast<const char*>(buffer), pos);
		// clear the buffer and reset pos value
		memset(buffer, 0, pos);
		pos = 0;
	}

	file.write("\0",1);
	file.close();
	return GPKIH_OK;
}

int db::profiles::initialize(size_t &profileCount){
	DEBUG(1, "db::profiles::initialize()");

	if(fs::exists(dbpath) == false){
		// Create database file
		std::ofstream file(dbpath, std::ios::binary);

		if(!file.is_open()){
			return GPKIH_FAIL;
		}

		mnck::dump(file, 0);
		file.close();

		size_t beg = 0;

		// Create id file
		std::ofstream idf(idfile, std::ios::binary);
		if(idf.is_open()){
			idf.write(reinterpret_cast<const char*>(&beg), sizeof(decltype(beg)));
			idf.close();
			return GPKIH_OK;
		}

		return GPKIH_FAIL;
	}

	// File already exists, load profiles
	std::ifstream file(dbpath, std::ios::binary);
	if(!file.is_open()){
		PERROR("couldn't open profile.data '{}'",dbpath);
		return GPKIH_FAIL;
	}

	if(fs::file_size(dbpath) == 0){
		file.close();
		return GPKIH_OK;
	}
	
	// Check magic number
	if(mnck::check(file, profileCount) == false){
		return GPKIH_FAIL;
	}

	size_t bufferSize = fs::file_size(dbpath) - 16;
	auto buffer = ALLOCATE(bufferSize);

	file.read(reinterpret_cast<char*>(buffer), bufferSize);
	file.close();

	DEBUGF(1, "Loading {} profiles", profileCount);

	bool unexistantSource = false;

	for(int i = 0;i < profileCount; ++i){
		try{
			Profile nProf{};
			memcpy(&nProf.meta, buffer, sizeof(ProfileMetadata));
			
			DEBUGF(1, "Profile metadata: [nameLen:{},sourceLen:{}]", nProf.meta.nameLen, nProf.meta.sourceLen);
			
			buffer+=sizeof(ProfileMetadata);
			
			nProf.name = reinterpret_cast<const char*>(buffer);
			buffer+=nProf.meta.nameLen+2;   // +2 to skip the null byte and point to next element (source)

			nProf.source = reinterpret_cast<const char*>(buffer);
			buffer+=nProf.meta.sourceLen+2; // +2 to skip the null byte and point to next element (next profile's id or null byte indicating EOF)
			
			if(fs::exists(nProf.source) == false){
				PWARN("Profile '{}' with root dir @ '{}' doesn't exist, might have been manually removed, removing from DB...\n", nProf.name, nProf.source);
				unexistantSource = true;			
				continue;
			}

			DEBUGF(1, "Loaded profile [name:{},source{}]", nProf.name, nProf.source, nProf.meta.nameLen, nProf.meta.sourceLen);
			
			const auto &[iter, success] = existing_profiles.emplace(nProf.name, nProf);
			
			if(success == false){
				throw("Failed loading profile map");
			}

		}catch(const std::exception &err){
			PERROR("Caught exception - {}\n", err.what());
			return GPKIH_FAIL;
		}
	}

	if(unexistantSource){
		DEBUG(1, "Syncing profile db file due to unexistant source");
		if(sync() == GPKIH_OK){
			PINFO("Succesfully removed unexistant profiles.\n");
		}
		return GPKIH_FATAL;
	}

	return GPKIH_OK;
};

bool db::profiles::exists(std::string_view profile_name) {
	return existing_profiles.find(profile_name.data()) != existing_profiles.end();
}

int db::profiles::add(Profile &buff){
	DEBUG(1, "db::profiles::add()");

	if (existing_profiles.find(buff.name) != existing_profiles.end()) {
		PERROR("Profile with name '{}' already exists\n", buff.name);
		return GPKIH_FAIL;
	}

	buff.meta.id = nextID();

	DEBUGF(3, "adding profile [id:{},name:{},source:{},caCreated:{},svCount:{},clCount:{},namelen:{},sourcelen:{}]", 
		buff.meta.id, buff.name, buff.source, buff.meta.caCreated, buff.meta.svCount, buff.meta.clCount, buff.meta.nameLen, buff.meta.sourceLen);

	existing_profiles.emplace(&buff.name[0], buff);
	return GPKIH_OK;
}

int db::profiles::remove(std::string_view profile_name) {
	auto iter = existing_profiles.find(profile_name.data());
	if (iter == existing_profiles.end()) {
		// profile not found
		return GPKIH_FAIL;
	}
	// profile found
	existing_profiles.erase(iter);
	return GPKIH_OK;
}

int db::profiles::remove(std::vector<std::string_view> profiles) {
	int deletedProfiles = 0;
	for(const auto &profile : profiles){
		auto iter = existing_profiles.find(profile.data());
		if (iter == existing_profiles.end()) {
			// profile not found
			return GPKIH_FAIL;
		}
		// profile found
		existing_profiles.erase(iter);
	}
	return deletedProfiles;
}

size_t db::profiles::remove_all(size_t *deletedFiles) {
	DEBUG(1, "db::profiles::remove_all()");
	int pcount = 0;
	size_t _deletedFiles = 0;
	for(auto &kv : existing_profiles){
		if(fs::exists(kv.second.source)){
			size_t n;
			n = fs::remove_all(kv.second.source);
			if(n == 0){
				PERROR("Couldn't remove profile '{}' path '{}'\n", kv.second.name, kv.second.source);
				continue;
			};

			_deletedFiles += n;
			n = fs::remove_all(fmt::format("{}{}_entities.data",GPKIH_DIR_DB,kv.first));
			
			if( n == 0){
				PERROR("Couldn't remove entity db for profile '{}'\n", kv.second.name);
				continue;
			};
			_deletedFiles += n;
			++pcount;
		}
	}

	existing_profiles.clear();

	if(deletedFiles != nullptr){
		*deletedFiles = _deletedFiles;
	}

	return GPKIH_OK;
}

int db::profiles::load(uint64_t profile_id, Profile &pinfo){
	for(const auto &[name, profile] : existing_profiles){
		if(profile.meta.id == profile_id){
			pinfo = profile;
			return GPKIH_OK;
		}
	}
	return GPKIH_FAIL;
}

int db::profiles::load(std::string_view profile_name, Profile &pinfo){
	if(existing_profiles.find(profile_name) != existing_profiles.end()){
		pinfo = existing_profiles[profile_name];
		return GPKIH_OK;
	}
	return GPKIH_FAIL;
}; // load profile named profile_name into struct Profile

Profile *const db::profiles::get(uint64_t profile_id){
	for(auto &[name, profile] : existing_profiles){
		if(profile.meta.id == profile_id){
			return &profile;
		}
	}
	return nullptr;
};

Profile *const db::profiles::get(std::string_view profile_name){
	auto iter = existing_profiles.find(profile_name);
	return (iter == existing_profiles.end() ? nullptr : &(iter->second));
}

size_t db::profiles::size(){
	return existing_profiles.size();
}