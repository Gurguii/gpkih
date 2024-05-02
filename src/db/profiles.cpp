#include "profiles.hpp"
#include <sstream>
#include <fstream> // std::ifstream | std::ofstream
#include <iostream> // std::cin

using namespace gpkih;

// Returns id from idfile and increases it, updating the file
static uint64_t get_id(){
	std::fstream file(db::profiles::idfile, std::ios::binary | std::ios::in | std::ios::out);
	
	if(!file.is_open()){
		seterror("couldn't open id file '{}' for reading\n", db::profiles::idfile);
		return GPKIH_FAIL;
	}

	uint64_t id = 0;
	file.read(reinterpret_cast<char*>(&id), sizeof(decltype(id)));

	if(file.fail()){
		seterror("couldn't read from id file\n");
		return GPKIH_FAIL;
	}

	uint64_t nid = id+1;
	file.seekp(SEEK_SET);
	file.write(reinterpret_cast<const char*>(&nid), sizeof(decltype(nid)));
	if(file.fail()){
		seterror("couldn't write to id file\n");
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

	std::ofstream file(dbpath, std::ios::binary | std::ios::out);
	if(!file.is_open()){
		seterror("couldn't open db file for writing '{}' - db::prosfiles::sync()\n", dbpath);
		return GPKIH_FAIL;
	}

	// _gpkih_23:<perfil1>%<perfil2>%...<perfil23>\0
	file.write(reinterpret_cast<const char*>(&gpkih_magic_number), sizeof(decltype(gpkih_magic_number)));
	size_t profile_count = size();
	file.write(reinterpret_cast<const char *>(&profile_count), sizeof(decltype(profile_count)));
	file.write(":",1);

	for(const auto &kv : existing_profiles){
		const Profile &p = kv.second;
		//printf("id: %lu\nname: %s\nsource: %s\ncreation: %lu\nmodification: %lu\nca_created: %d\nsv_count: %d\ncl_count: %d\n", p.id, p.name, p.source, p.creation_date, p.last_modification, p.ca_created, p.sv_count, p.cl_count);
		//fflush(stdout);
		// kv.first ->  std::string_view - profile name
		// kv.second -> Profile - profile struct 
		const Profile &ref = kv.second;
		file.write(reinterpret_cast<const char*>(&ref.id), sizeof(decltype(ref.id)));

		file.write(reinterpret_cast<const char*>(&ref.namelen), sizeof(decltype(ref.namelen)));
		file.write(ref.name, ref.namelen);
		
		file.write(reinterpret_cast<const char *>(&ref.sourcelen), sizeof(decltype(ref.sourcelen)));
		file.write(ref.source, ref.sourcelen);	
		
		file.write(reinterpret_cast<const char*>(&ref.creation_date), sizeof(decltype(ref.creation_date)));

		file.write(reinterpret_cast<const char*>(&ref.last_modification), sizeof(decltype(ref.last_modification)));

		file.write(reinterpret_cast<const char*>(&ref.ca_created), sizeof(decltype(ref.ca_created)));

		file.write(reinterpret_cast<const char*>(&ref.sv_count), sizeof(decltype(ref.sv_count)));

		file.write(reinterpret_cast<const char*>(&ref.cl_count), sizeof(decltype(ref.cl_count)));

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
		auto file = std::ofstream(dbpath,std::ios::binary);
		if(!file.is_open()){
			seterror("couldn't create db file '{}'", dbpath);
			return GPKIH_FAIL;	
		}
		file.write(reinterpret_cast<const char*>(&gpkih_magic_number), sizeof(decltype(gpkih_magic_number)));
		size_t n = 0;
		file.write(reinterpret_cast<const char*>(&n), sizeof(decltype(n)));
		file.write(":",1);
		file.close();

		// Create id_tracking file
		uint64_t starting = 0;
		if(!std::ofstream(idfile,std::ios::binary).write(reinterpret_cast<const char*>(&starting), sizeof(decltype(starting))).good()){
			seterror("couldn't create missing file '{}'", idfile);
			return GPKIH_FAIL;
		}

		return GPKIH_OK;
	}

	// File already exists, load profiles
	// syntax: <id><namelen><name><sourcelen><source><creation_date><last_modification><ca><total_servers><total_clients>
	std::ifstream file(dbpath, std::ios::binary);
	if(!file.is_open()){
		seterror("couldn't open profile.data '{}'",dbpath);
		return GPKIH_FAIL;
	}

	if(fs::file_size(dbpath) == 0){
		file.close();
		return GPKIH_OK;
	}
	
	// Check magic number
	size_t mn = 0;
	file.read(reinterpret_cast<char *>(&mn), sizeof(decltype(gpkih_magic_number)));
	if(mn == gpkih_magic_number){
		file.read(reinterpret_cast<char*>(&loaded_profiles), sizeof(decltype(loaded_profiles)));
		if(loaded_profiles <= 0){
			return GPKIH_OK;
		}
		uint8_t next = file.get();
		if(next != ':'){
			PWARN("file doesn't seem like a 'gpkih' data file");
			return GPKIH_OK;
		}
	}else{
		PWARN("file doesn't seem a 'gpkih' data file\n");
		return GPKIH_OK;
	}

	for(int i = 0;i < loaded_profiles; ++i){
		Profile p;
		file.read(reinterpret_cast<char*>(&p.id), sizeof(decltype(p.id)));
		
		file.read(reinterpret_cast<char*>(&p.namelen), sizeof(decltype(p.namelen)));

		p.name = ALLOCATE(p.namelen);
		if(p.name == NULL){
			seterror("couldn't allocate memory for profile name\n");
			return GPKIH_FAIL;
		}
		file.read(p.name, p.namelen);

		file.read(reinterpret_cast<char*>(&p.sourcelen), sizeof(decltype(p.sourcelen)));
		p.source = ALLOCATE(p.sourcelen);
		if(p.source == NULL){
			seterror("couldn't allocate memory for profile source\n");
			return GPKIH_FAIL;
		}
		file.read(p.source, p.sourcelen);

		file.read(reinterpret_cast<char*>(&p.creation_date), sizeof(decltype(p.creation_date)));
		file.read(reinterpret_cast<char*>(&p.last_modification), sizeof(decltype(p.last_modification)));
		file.read(reinterpret_cast<char*>(&p.ca_created), sizeof(decltype(p.ca_created)));
		file.read(reinterpret_cast<char*>(&p.sv_count), sizeof(decltype(p.sv_count)));
		file.read(reinterpret_cast<char*>(&p.cl_count), sizeof(decltype(p.cl_count)));

		// Add profile to map
     	existing_profiles.emplace(p.name, std::move(p));

     	PDEBUG(2,"loaded profile '{}'",p.name);
     	uint8_t next = file.get();
     	if(next != '%'){
     		PWARN("IM OUT BITCH");
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
		seterror("profile with name '{}' already exists", buff.name);
		return GPKIH_FAIL;
	}
	PDEBUG(3, "adding profile [name:{},source:{},ca_created:{},sv_count:{},cl_count:{}]", buff.name, buff.source, buff.ca_created, buff.sv_count, buff.cl_count);

	buff.id = get_id();
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

int db::profiles::remove_all() {
	PDEBUG(1, "db::profiles::remove_all()");
	int pcount = 0;

	for(auto &kv : existing_profiles){
		if(fs::exists(kv.second.source)){
			PDEBUG(3, "deleting source file - {}",kv.second.source);
			if(fs::remove_all(kv.second.source) > 0 && fs::remove_all(fmt::format("{}{}_entities.data",DB_DIRPATH,kv.first))){
				++pcount;
			}
		}
	}

	existing_profiles.clear();
	fs::remove(dbpath);
	
	return pcount;
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