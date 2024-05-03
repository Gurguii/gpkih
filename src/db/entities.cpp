#include "entities.hpp"
#include <cstdio>
#include <cstring>
#include <exception>
#include <filesystem>
#include <sstream> // std::stringstream
#include <fstream> // std::ifstream | std::ofstream
#include <thread>
#include <unordered_map>


#include <unistd.h>
using namespace gpkih;

static inline std::string _dbpath(std::string_view profile) {
  return fmt::format("{}{}_entities.data",EntityManager::db,profile);
}

EntityManager::EntityManager(std::string_view profile_name){
	dbpath = _dbpath(profile_name);
	PDEBUG(1, "constructing EntityManager instance - {}", dbpath);

	if(fs::exists(dbpath) == false){
		std::ofstream file(dbpath, std::ios::binary);
		if(!file.is_open()){
			throw "couldn't create entities' dbpath";
		}
		file.write(reinterpret_cast<char *>(gpkih_magic_number), sizeof(decltype(gpkih_magic_number)));
		size_t st = 0;
		file.write(reinterpret_cast<const char*>(&st), sizeof(decltype(st)));
		file.write(":",1);
		file.close();
		return;
	}
	
	if(fs::is_empty(dbpath)){
		return;	
	}

	// Load entities
	std::ifstream file(dbpath, std::ios::binary);

	if(!file.is_open()){
		throw "couldn't open entities' dbpath to load data";
	}

	// Check magic number
	size_t mn = 0, entity_count = 0;
	file.read(reinterpret_cast<char *>(&mn), sizeof(decltype(gpkih_magic_number)));
	if(mn == gpkih_magic_number){
		file.read(reinterpret_cast<char*>(&entity_count), sizeof(decltype(entity_count)));
		if(entity_count <= 0){
			return;
		}
		uint8_t next = file.get();
		if(next != ':'){
			PWARN("file doesn't seem like a 'gpkih' data file");
			return;
		}
	}else{
		PWARN("file doesn't seem a 'gpkih' data file\n");
		return;
	}

	for(int i = 0;i < entity_count; ++i){
		Entity tmp{};
		Subject &sub = tmp.subject;

		file.read(reinterpret_cast<char*>(&tmp.serial), sizeof(decltype(Entity::serial)));

		file.read(reinterpret_cast<char*>(&sub.cnlen), sizeof(decltype(Subject::cnlen)));
		if((sub.cn = ALLOCATE(sub.cnlen)) == NULL){
			break;
		}
		file.read(sub.cn, sub.cnlen);

		file.read(reinterpret_cast<char*>(&tmp.type), sizeof(decltype(Entity::type)));

		file.read(reinterpret_cast<char*>(&tmp.creation_date), sizeof(decltype(Entity::creation_date)));

		file.read(sub.country, sizeof(sub.country));

		file.read(reinterpret_cast<char*>(&sub.locationlen), sizeof(decltype(Subject::locationlen)));
		if((sub.location = ALLOCATE(sub.locationlen)) == NULL){
			break;
		}
		file.read(sub.location, sub.locationlen);

		file.read(reinterpret_cast<char*>(&sub.organisationlen), sizeof(decltype(Subject::organisationlen)));
		if((sub.organisation = ALLOCATE(sub.organisationlen)) == NULL){
			break;
		}
		file.read(sub.organisation, sub.organisationlen);
	
		file.read(reinterpret_cast<char*>(&sub.statelen), sizeof(decltype(Subject::statelen)));
		if((sub.state = ALLOCATE(sub.statelen)) == NULL){
			break;
		}
		file.read(sub.state, sub.statelen);

		file.read(reinterpret_cast<char*>(&sub.emaillen), sizeof(decltype(Subject::emaillen)));
		if((sub.email = ALLOCATE(sub.emaillen)) == NULL){
			break;
		}
		file.read(sub.email, sub.emaillen);
	
		file.read(reinterpret_cast<char*>(&tmp.key_path_len), sizeof(decltype(Entity::key_path_len)));
		if((tmp.key_path = ALLOCATE(tmp.key_path_len)) == NULL){
			break;
		}
		file.read(tmp.key_path, tmp.key_path_len);

		file.read(reinterpret_cast<char*>(&tmp.csr_path_len), sizeof(decltype(Entity::csr_path_len)));
		if((tmp.csr_path = ALLOCATE(tmp.csr_path_len)) == NULL){
			break;
		}
		file.read(tmp.csr_path, tmp.csr_path_len);

		file.read(reinterpret_cast<char*>(&tmp.crt_path_len), sizeof(decltype(Entity::crt_path_len)));
		if((tmp.crt_path = ALLOCATE(tmp.crt_path_len) )== NULL){
			break;
		}
		file.read(tmp.crt_path, tmp.crt_path_len);

		// Add entity to umap
		entities.emplace(sub.cn, tmp);
	}
}

int EntityManager::sync(){
	PDEBUG(1, "EntityManager::sync()");

	std::ofstream file(dbpath, std::ios::binary);
	if(!file.is_open()){
		seterror("couldn't open entities db '{}'",dbpath);
		return GPKIH_FAIL;
	}

	// _gpkih_23:<entidad1>%<entidad2>%...<entidad23>\0
	file.write(reinterpret_cast<const char*>(&gpkih_magic_number), sizeof(decltype(gpkih_magic_number)));
	size_t entity_count = entities.size();
	file.write(reinterpret_cast<const char *>(&entity_count), sizeof(decltype(entity_count)));
	file.write(":",1);
	PDEBUG(1, "entities size: {}", entity_count);

	for(const auto &kv : entities){
		const gpkih::Entity &entity = kv.second;
		const gpkih::Subject &subject = kv.second.subject;

		PDEBUG(1, "adding entity to file [serial = {}, cn = {}:{}, co = {}:{}, st = {}:{}, type = {}]",entity.serial,entity.subject.cn,subject.cnlen, subject.country,3, subject.state,subject.statelen, str_conversion(entity.type));
		file.write(reinterpret_cast<const char*>(&entity.serial),sizeof(decltype(entity.serial)));
		
		file.write(reinterpret_cast<const char*>(&subject.cnlen), sizeof(decltype(subject.cnlen)));
		file.write(subject.cn,subject.cnlen);
	
		file.write(reinterpret_cast<const char*>(&entity.type), sizeof(decltype(entity.type)));
	
		file.write(reinterpret_cast<const char*>(&entity.creation_date), sizeof(decltype(entity.creation_date)));
	
		file.write(subject.country,sizeof(subject.country));
	
		file.write(reinterpret_cast<const char*>(&subject.locationlen),sizeof(decltype(subject.locationlen)));
		//PDEBUG(3, "subject.location = {}\n", subject.location);
		file.write(subject.location,subject.locationlen);
	
		file.write(reinterpret_cast<const char *>(&subject.organisationlen),sizeof(decltype(subject.organisationlen)));
		//PDEBUG(3, "subject.organisation = {} len = {}\n", subject.organisation, subject.organisationlen);
		file.write(subject.organisation,subject.organisationlen);
	
		file.write(reinterpret_cast<const char*>(&subject.statelen), sizeof(decltype(subject.statelen)));
		file.write(subject.state,subject.statelen);
	
		file.write(reinterpret_cast<const char*>(&subject.emaillen), sizeof(decltype(subject.emaillen)));
		file.write(entity.subject.email,entity.subject.emaillen);
	
		file.write(reinterpret_cast<const char*>(&entity.key_path_len),sizeof(decltype(entity.key_path_len)));
		file.write(entity.key_path,entity.key_path_len);
	
		file.write(reinterpret_cast<const char*>(&entity.csr_path_len), sizeof(decltype(entity.csr_path_len)));
		file.write(entity.csr_path,entity.csr_path_len);
	
		file.write(reinterpret_cast<const char*>(&entity.crt_path_len),sizeof(decltype(entity.crt_path_len)));
		file.write(entity.crt_path,entity.crt_path_len);
	
		file.write("%", 1);
	}

	file.write("\0", 1);
	file.close();

	return GPKIH_OK;
}

int EntityManager::add(gpkih::Entity &entity){
	PDEBUG(1, "EntityManager::add()");
	PDEBUG(1, "{}-{}-{:%h-%m-%Y}-{}-{}-{}",entity.serial, entity.subject.cn, entity.creation_date, entity.subject.country,entity.subject.state, entity.subject.organisation);

	if(entities.find(entity.subject.cn) == entities.end()){
		return (entities.emplace(entity.subject.cn, entity).second) ? GPKIH_OK : GPKIH_FAIL;
	}

	return GPKIH_FAIL;
}

bool EntityManager::exists(std::string_view cn){
	return entities.find(cn) != entities.end() ? true : false;
}

bool EntityManager::exists(size_t serial){
	PDEBUG(3, "looking for serial {}", serial);
	for(const auto &kv : entities){
		PDEBUG(3, "checking {}", kv.second.serial);
		if(kv.second.serial == serial){
			return true;
		}
	}
	return false;
}

int EntityManager::del(std::string_view cn){
	PDEBUG(1, "EntityManager::del()");

	auto iter = entities.find(cn);
	if( iter != entities.end()){
		entities.erase(iter);
		return GPKIH_OK;
	}
	return GPKIH_FAIL;
}

gpkih::Entity *const EntityManager::get(std::string_view cn){
	PDEBUG(1, "EntityManager::get()");

	auto iter = entities.find(cn);
	if(iter != entities.end()){
		return &(iter->second);
	}
	return NULL;
}

size_t EntityManager::size(){
	return entities.size();
}

const std::unordered_map<std::string_view, gpkih::Entity>* const EntityManager::retrieve(){
	return &entities;
}