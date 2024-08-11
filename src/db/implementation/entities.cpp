#include "../entities.hpp"
#include "../mnck.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../gpkih.hpp"
#include "../../entities/entities.hpp"
#include "../../entities/conv.hpp"
#include <cstddef>

using namespace gpkih;

constexpr size_t kEntitySafeBufferSize = sizeof(EntityMetadata) + sizeof(SubjectMetadata) + (GPKIH_MAX_PATH * 3)
+ (GPKIH_MAX_VARCHAR * 5) + sizeof(Subject::country) + 9;

const std::string& EntityManager::setDir(std::string_view path){
	dbDir = path;
	return dbDir;
};

const std::string& EntityManager::getDir(){
	return dbDir;
}

EntityManager::EntityManager(std::string_view profileName)
:dbpath(std::string{dbDir} + profileName.data() + "_entities.data"){
	DEBUGF(1, "EntityManager::EntityManager({})", dbpath);

	if(fs::exists(dbpath) == false){
		std::ofstream file(dbpath, std::ios::binary);
		
		if(!file.is_open()){
			PERROR("Couldn't create entities' dbpath\n");
			return;
		}

		mnck::dump(file, 0);
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

	size_t entityCount = 0;

	// Check magic number
	if(mnck::check(file, entityCount) == false){
		DEBUG(2,"Failed magic number check");
		return;
	}

	DEBUGF(2, "Loading {} entities", entityCount);

	if(entityCount == 0){
		return;
	}

	size_t bufferSize = fs::file_size(dbpath) - 16;
	auto buffer = ALLOCATE(bufferSize);

	if(buffer == nullptr){
		throw(gpkihBuffer->getLastError());
	}

	file.read(reinterpret_cast<char*>(buffer), bufferSize);
	file.close();

	auto current = buffer;
	for(int i = 0;i < entityCount; ++i){
		Entity nEnt{};
		Subject &sub = nEnt.subject;

		nEnt.meta = *(reinterpret_cast<EntityMetadata*>(current));
		current+=sizeof(EntityMetadata);

		sub.meta = *(reinterpret_cast<SubjectMetadata*>(current));
		current+=sizeof(SubjectMetadata);

		DEBUGF(3, "Loading entity [serial:{},creationDate:{:%d-%m-%Y @ %H:%M},expirationDate:{:%d-%m-%Y @ %H:%M}]", nEnt.meta.serial, nEnt.meta.creationDate, nEnt.meta.expirationDate);

		memcpy(const_cast<char*>(sub.country), current, sizeof(sub.country));
		current+=sizeof(sub.country)+1;

		sub.cn = reinterpret_cast<const char*>(current);
		current+=sub.meta.cnlen+1;

		DEBUGF(3, "Country: {} CN: {}", sub.country, sub.cn);
		// state location organisation email key csr crt
		sub.state = reinterpret_cast<const char*>(current); 
		current+=sub.meta.statelen + 1;

		sub.location = reinterpret_cast<const char*>(current);
		current+=sub.meta.locationlen + 1;

		sub.organisation = reinterpret_cast<const char*>(current);
		current+=sub.meta.organisationlen + 1;

		sub.email = reinterpret_cast<const char*>(current);
		current+=sub.meta.emaillen + 1;

		nEnt.keyPath = reinterpret_cast<const char*>(current);
		current+=nEnt.meta.keyPathLen + 1;

		nEnt.csrPath = reinterpret_cast<const char*>(current);
		current+=nEnt.meta.csrPathLen + 1;

		nEnt.crtPath = reinterpret_cast<const char*>(current);
		current+=nEnt.meta.crtPathLen + 1;

		DEBUGF(3, "Loaded entity [cn:{},key:{},serial:{},type:{}] ", nEnt.subject.cn, nEnt.keyPath, nEnt.meta.serial,entity::conversion::toString(nEnt.meta.type));

		// Add entity to umap
		const auto &[iter, success] = entities.emplace(sub.cn, nEnt);
		if(success == false){
			PERROR("couldn't emplace entity [serial:{},cn:{}]",nEnt.meta.serial,sub.cn);
			continue;
		}
	}
}

int EntityManager::sync(){
	DEBUG(1, "EntityManager::sync()");

	std::ofstream file(dbpath, std::ios::binary);
	if(!file.is_open()){
		PERROR("couldn't open entities db '{}'",dbpath);
		return GPKIH_FAIL;
	}

	size_t entitiesToWrite = entities.size();
	if(mnck::dump(file, entitiesToWrite) == false){
		DEBUGF(2,"File '{}' does not have a proper magic number", dbpath);
		return GPKIH_FAIL;
	}

	DEBUGF(2, "Writing '{}' entities", entitiesToWrite);
	gurgui::memory::SmartMemBlock buffer{kEntitySafeBufferSize};
	auto buff = buffer.get();

	if(buff == nullptr){
		PERROR("Failed to allocate memory for buffer");
		return GPKIH_FAIL;
	}

	int pos = 0;
	for(const auto &[cn, entity] : entities){
		// Load entity to buffer and write to file with 1 write call
		memcpy(buff + pos, &entity.meta, sizeof(EntityMetadata));
		pos += sizeof(EntityMetadata);
		
		memcpy(buff + pos, &entity.subject.meta, sizeof(SubjectMetadata));
		pos += sizeof(SubjectMetadata);
		/////////7
		memcpy(buff + pos, entity.subject.country, sizeof(entity.subject.country));
		pos += sizeof(entity.subject.country) + 1;
		
		memcpy(buff + pos, entity.subject.cn, entity.subject.meta.cnlen);
		pos += entity.subject.meta.cnlen + 1;
		
		memcpy(buff + pos, entity.subject.state, entity.subject.meta.statelen);
		pos += entity.subject.meta.statelen + 1;

		memcpy(buff + pos, entity.subject.location, entity.subject.meta.locationlen);
		pos += entity.subject.meta.locationlen + 1;
		
		memcpy(buff + pos, entity.subject.organisation, entity.subject.meta.organisationlen);
		pos += entity.subject.meta.organisationlen + 1;

		memcpy(buff + pos, entity.subject.email, entity.subject.meta.emaillen);
		pos += entity.subject.meta.emaillen + 1;
		
		memcpy(buff + pos, entity.keyPath, entity.meta.keyPathLen);
		pos += entity.meta.keyPathLen + 1;
		
		memcpy(buff + pos, entity.csrPath, entity.meta.csrPathLen);
		pos += entity.meta.csrPathLen + 1;
		
		memcpy(buff + pos, entity.crtPath, entity.meta.crtPathLen);
		pos += entity.meta.crtPathLen + 1;

		file.write(reinterpret_cast<const char*>(buff), pos);

		memset(buff, 0, pos);
		pos = 0;
	}

	file.write("\0", 1);
	file.close();

	return GPKIH_OK;
}

int EntityManager::add(Entity &entity){
	DEBUG(1, "EntityManager::add()");

	if(entities.find(entity.subject.cn) == entities.end()){
		const auto [iter, success] = entities.emplace(entity.subject.cn, entity);
		if(success){
			return entities.size();
		}
	}

	return GPKIH_FAIL;
}

bool EntityManager::exists(std::string_view cn){
	DEBUGF(1,"EntityManager::exists({})",cn);
	return entities.find(cn) != entities.end();
}

bool EntityManager::exists(size_t serial){
	DEBUGF(1, "EntityManager::exists({})", serial);

	for(const auto &[cn, entity] : entities){
		if(entity.meta.serial == serial){
			return true;
		}
	}
	return false;
}

bool EntityManager::exists(std::string_view cn, Entity *&buff){
	DEBUGF(1, "EntityManager::exists({})",cn);
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
	DEBUG(1, "EntityManager::del()");
	auto iter = entities.find(cn);
	if( iter != entities.end()){
		entities.erase(iter);
		return GPKIH_OK;
	}
	return GPKIH_FAIL;
}

Entity *const EntityManager::get(std::string_view cn){
	DEBUG(1, "EntityManager::get()");
	auto iter = entities.find(cn);
	if(iter != entities.end()){
		return &(iter->second);
	}
	return nullptr;
}

size_t EntityManager::size(){
	DEBUG(1, "EntityManager::size()");
	return entities.size();
}

bool EntityManager::empty(){
	DEBUG(1, "EntityManager::empty()");
	return entities.empty();
}

const std::map<std::string_view, Entity>* const EntityManager::retrieve(){
	DEBUG(1, "EntityManager::retrieve()");
	return &entities;
}