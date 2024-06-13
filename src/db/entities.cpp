#include "entities.hpp"
#include "mnck.hpp"
#include "../memory/memmgmt.hpp"
#include <stdexcept>

using namespace gpkih;

static inline std::string _dbpath(std::string_view profile) {
  return fmt::format("{}{}_entities.data",EntityManager::dbdir,profile);
}

EntityManager::EntityManager(std::string_view profile_name){
	dbpath = _dbpath(profile_name);
	PDEBUG(1, "EntityManager::EntityManager({})", dbpath);

	if(fs::exists(dbpath) == false){
		std::ofstream file(dbpath, std::ios::binary);
		
		if(!file.is_open()){
			PERROR("Couldn't create entities' dbpath\n");
			return;
		}

		mnck::dump(file, current_size);
		file.close();
		return;
	}
	
	if(fs::is_empty(dbpath)){
		return;	
	}

	// Load entities
	std::ifstream file(dbpath, std::ios::binary);

	if(!file.is_open()){
		PERROR("Couldn't open entities' dbpath to load data\n");
		return;
	}

	// Check magic number
	if(mnck::check(file, current_size) == false){
		PDEBUG(2,"Failed magic number check");
		return;
	}

	PDEBUG(2, "Loading {} entities", current_size);
	for(int i = 0;i < current_size; ++i){
		Entity tmp{};
		Subject &sub = tmp.subject;

		file.read(reinterpret_cast<char*>(&tmp.serial), sizeof(decltype(Entity::serial)));

		file.read(reinterpret_cast<char*>(&sub.cnlen), sizeof(decltype(Subject::cnlen)));
		if((sub.cn = ALLOCATE(sub.cnlen)) == nullptr){
			break;
		}
		file.read(sub.cn, sub.cnlen);

		file.read(reinterpret_cast<char*>(&tmp.type), sizeof(decltype(Entity::type)));

		file.read(reinterpret_cast<char*>(&tmp.creationDate), sizeof(decltype(Entity::creationDate)));
		file.read(reinterpret_cast<char*>(&tmp.expirationDate), sizeof(decltype(Entity::expirationDate)));
		
		file.read(reinterpret_cast<char*>(&tmp.status), sizeof(decltype(Entity::status)));

		file.read(sub.country, sizeof(sub.country));

		file.read(reinterpret_cast<char*>(&sub.locationlen), sizeof(decltype(Subject::locationlen)));
		if((sub.location = ALLOCATE(sub.locationlen)) == nullptr){
			break;
		}
		file.read(sub.location, sub.locationlen);

		file.read(reinterpret_cast<char*>(&sub.organisationlen), sizeof(decltype(Subject::organisationlen)));
		if((sub.organisation = ALLOCATE(sub.organisationlen)) == nullptr){
			break;
		}
		file.read(sub.organisation, sub.organisationlen);
	
		file.read(reinterpret_cast<char*>(&sub.statelen), sizeof(decltype(Subject::statelen)));
		if((sub.state = ALLOCATE(sub.statelen)) == nullptr){
			break;
		}
		file.read(sub.state, sub.statelen);

		file.read(reinterpret_cast<char*>(&sub.emaillen), sizeof(decltype(Subject::emaillen)));
		if((sub.email = ALLOCATE(sub.emaillen)) == nullptr){
			break;
		}
		file.read(sub.email, sub.emaillen);
	
		file.read(reinterpret_cast<char*>(&tmp.keyPathLen), sizeof(decltype(Entity::keyPathLen)));
		if((tmp.keyPath = ALLOCATE(tmp.keyPathLen)) == nullptr){
			break;
		}
		file.read(tmp.keyPath, tmp.keyPathLen);

		file.read(reinterpret_cast<char*>(&tmp.csrPathLen), sizeof(decltype(Entity::csrPathLen)));
		if((tmp.csrPath = ALLOCATE(tmp.csrPathLen)) == nullptr){
			break;
		}
		file.read(tmp.csrPath, tmp.csrPathLen);

		file.read(reinterpret_cast<char*>(&tmp.crtPathLen), sizeof(decltype(Entity::crtPathLen)));
		if((tmp.crtPath = ALLOCATE(tmp.crtPathLen) )== nullptr){
			break;
		}
		file.read(tmp.crtPath, tmp.crtPathLen);

		// Add entity to umap
		const auto [iter, success] = entities.emplace(sub.cn, tmp);
		if(success == false){
			PERROR("couldn't emplace entity [serial:{},cn:{}]",tmp.serial,sub.cn);
			continue;
		}
		PDEBUG(3, "Loaded entity {} - {} - {}", tmp.subject.cn, tmp.keyPath, tmp.serial);
		uint8_t next = file.get();
		if(next != '%'){
			break;
		}		
	}
	
	file.close();
}

int EntityManager::sync(){
	PDEBUG(1, "EntityManager::sync()");

	std::ofstream file(dbpath, std::ios::binary);
	if(!file.is_open()){
		PERROR("couldn't open entities db '{}'",dbpath);
		return GPKIH_FAIL;
	}

	if(mnck::dump(file, current_size) == false){
		PDEBUG(2,"File '{}' does not have a proper magic number");
		return GPKIH_FAIL;
	}

	for(const auto &kv : entities){
		PDEBUG(3,"Writing entity {} {}", kv.second.subject.cn, kv.second.serial);

		const Entity &entity = kv.second;
		const Subject &subject = kv.second.subject;

		file.write(reinterpret_cast<const char*>(&entity.serial),sizeof(decltype(entity.serial)));
		
		file.write(reinterpret_cast<const char*>(&subject.cnlen), sizeof(decltype(subject.cnlen)));
		file.write(subject.cn,subject.cnlen);
	
		file.write(reinterpret_cast<const char*>(&entity.type), sizeof(decltype(entity.type)));
	
		file.write(reinterpret_cast<const char*>(&entity.creationDate), sizeof(decltype(entity.creationDate)));
		file.write(reinterpret_cast<const char *>(&entity.expirationDate), sizeof(decltype(entity.expirationDate)));

		file.write(reinterpret_cast<const char*>(&entity.status), sizeof(decltype(entity.status)));

		file.write(subject.country,sizeof(subject.country));
	
		file.write(reinterpret_cast<const char*>(&subject.locationlen),sizeof(decltype(subject.locationlen)));

		file.write(subject.location,subject.locationlen);
	
		file.write(reinterpret_cast<const char *>(&subject.organisationlen),sizeof(decltype(subject.organisationlen)));
		file.write(subject.organisation,subject.organisationlen);
	
		file.write(reinterpret_cast<const char*>(&subject.statelen), sizeof(decltype(subject.statelen)));
		file.write(subject.state,subject.statelen);
	
		file.write(reinterpret_cast<const char*>(&subject.emaillen), sizeof(decltype(subject.emaillen)));
		file.write(entity.subject.email,entity.subject.emaillen);
	
		file.write(reinterpret_cast<const char*>(&entity.keyPathLen),sizeof(decltype(entity.keyPathLen)));
		file.write(entity.keyPath,entity.keyPathLen);
	
		file.write(reinterpret_cast<const char*>(&entity.csrPathLen), sizeof(decltype(entity.csrPathLen)));
		file.write(entity.csrPath,entity.csrPathLen);
	
		file.write(reinterpret_cast<const char*>(&entity.crtPathLen),sizeof(decltype(entity.crtPathLen)));
		file.write(entity.crtPath,entity.crtPathLen);
	
		file.write("%", 1);
	}

	file.write("\0", 1);
	file.close();

	return GPKIH_OK;
}

int EntityManager::add(gpkih::Entity &entity){
	PDEBUG(1, "EntityManager::add()");

	if(entities.find(entity.subject.cn) == entities.end()){
		const auto [iter, success] = entities.emplace(entity.subject.cn, entity);
		if(success){
			return ++current_size;
		}
	}

	return GPKIH_FAIL;
}

bool EntityManager::exists(std::string_view cn){
	PDEBUG(1,"EntityManager::exists({})",cn);

	return entities.find(cn) != entities.end();
}

bool EntityManager::exists(size_t serial){
	PDEBUG(1, "EntityManager::exists({})", serial);

	for(const auto &kv : entities){
		if(kv.second.serial == serial){
			return true;
		}
	}
	return false;
}

bool EntityManager::exists(std::string_view cn, Entity *&buff){
	PDEBUG(1, "EntityManager::exists({})",cn);
	auto iter = entities.find(cn);
	if(iter != entities.end()){
		// It exists
		buff = &(iter->second);
		return true;
	}
	buff = nullptr;
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
	return nullptr;
}

size_t EntityManager::size(){
	PDEBUG(1, "EntityManager::size()");
	return current_size;
}

bool EntityManager::empty(){
	PDEBUG(1, "EntityManager::empty()");
	return current_size == 0;
}

const std::map<std::string_view, gpkih::Entity>* const EntityManager::retrieve(){
	PDEBUG(1, "EntityManager::retrieve()");
	return &entities;
}