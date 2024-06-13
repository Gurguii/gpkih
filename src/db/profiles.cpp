#include "profiles.hpp"
#include <fstream> // std::ifstream | std::ofstream
#include <iostream> // std::cin
#include "../printing/printing.hpp"
#include "../memory/memmgmt.hpp"
#include "mnck.hpp"

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

static std::map <std::string_view, Profile > existing_profiles{};

std::map <std::string_view, Profile>* const db::profiles::get(){
	return &existing_profiles;
}

// overwrite dbpath (data.bin) with the profiles in existing_profiles
int db::profiles::sync(){
	PDEBUG(1,"db::profiles::sync()");

	std::ofstream file(dbpath, std::ios::binary);
	if(!file.is_open()){
		PERROR("couldn't open db file for writing '{}' - db::prosfiles::sync()\n", dbpath);
		return GPKIH_FAIL;
	}

	size_t s = size();
	if(mnck::dump(file,s) == false){
		return GPKIH_FAIL;
	};

	for(const auto &kv : existing_profiles){
		const Profile &p = kv.second;

		file.write(reinterpret_cast<const char*>(&p.id), sizeof(decltype(p.id)));

		file.write(reinterpret_cast<const char*>(&p.namelen), sizeof(decltype(p.namelen)));
		file.write(p.name, p.namelen);
		
		file.write(reinterpret_cast<const char *>(&p.sourcelen), sizeof(decltype(p.sourcelen)));
		file.write(p.source, p.sourcelen);	
		
		file.write(reinterpret_cast<const char*>(&p.creationDate), sizeof(decltype(p.creationDate)));

		file.write(reinterpret_cast<const char*>(&p.last_modification), sizeof(decltype(p.last_modification)));

		file.write(reinterpret_cast<const char*>(&p.ca_created), sizeof(decltype(p.ca_created)));

		file.write(reinterpret_cast<const char*>(&p.sv_count), sizeof(decltype(p.sv_count)));

		file.write(reinterpret_cast<const char*>(&p.cl_count), sizeof(decltype(p.cl_count)));

		file.write("%",1);
	}

	file.write("\0",1);
	file.close();
	return GPKIH_OK;
}

int db::profiles::initialize(size_t &loaded_profiles){
	PDEBUG(1, "db::profiles::initialize()");

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
		}else{
			return GPKIH_FAIL;
		};
	}

	// File already exists, load profiles
	// syntax: <id><namelen><name><sourcelen><source><creationDate><last_modification><ca><total_servers><total_clients>
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
	if(mnck::check(file, loaded_profiles) == false){
		return GPKIH_FAIL;
	}

	for(int i = 0;i < loaded_profiles; ++i){
		Profile p;
		file.read(reinterpret_cast<char*>(&p.id), sizeof(decltype(p.id)));
		
		file.read(reinterpret_cast<char*>(&p.namelen), sizeof(decltype(p.namelen)));

		p.name = ALLOCATE(p.namelen);
		if(p.name == nullptr){
			PERROR("couldn't allocate memory for profile name\n");
			return GPKIH_FAIL;
		}
		file.read(p.name, p.namelen);

		file.read(reinterpret_cast<char*>(&p.sourcelen), sizeof(decltype(p.sourcelen)));
		p.source = ALLOCATE(p.sourcelen);
		if(p.source == nullptr){
			PERROR("couldn't allocate memory for profile source\n");
			return GPKIH_FAIL;
		}
		file.read(p.source, p.sourcelen);

		file.read(reinterpret_cast<char*>(&p.creationDate), sizeof(decltype(p.creationDate)));
		file.read(reinterpret_cast<char*>(&p.last_modification), sizeof(decltype(p.last_modification)));
		file.read(reinterpret_cast<char*>(&p.ca_created), sizeof(decltype(p.ca_created)));
		file.read(reinterpret_cast<char*>(&p.sv_count), sizeof(decltype(p.sv_count)));
		file.read(reinterpret_cast<char*>(&p.cl_count), sizeof(decltype(p.cl_count)));

		// Add profile to map
     	existing_profiles.emplace(p.name, std::move(p));
     	
     	PDEBUG(2,"loaded profile '{}'",p.name);
     	uint8_t next = file.get();
     	if(next != '%'){
     		break;
     	}
	}
	file.close();
	
	return GPKIH_OK;
};

bool db::profiles::exists(std::string_view profile_name) {
	return existing_profiles.find(profile_name.data()) != existing_profiles.end();
}

int db::profiles::add(Profile &buff){
	PDEBUG(1, "db::profiles::add()");
	if (exists(buff.name) == true) {
		PERROR("profile with name '{}' already exists", buff.name);
		return GPKIH_FAIL;
	}
	

	buff.id = nextID();

	PDEBUG(3, "adding profile [id:{},name:{},source:{},ca_created:{},sv_count:{},cl_count:{}]", buff.id, buff.name, buff.source, buff.ca_created, buff.sv_count, buff.cl_count);

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

size_t db::profiles::remove_all(size_t *deletedFiles) {
	PDEBUG(1, "db::profiles::remove_all()");
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
			n = fs::remove_all(fmt::format("{}{}_entities.data",DB_DIRPATH,kv.first));
			
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
	for(const auto &kv : existing_profiles){
		if(kv.second.id == profile_id){
			pinfo = kv.second;
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
	for(auto &kv : existing_profiles){
		if(kv.second.id == profile_id){
			return &kv.second;
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