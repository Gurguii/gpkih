#include "export.hpp"

#include "../../../db/profiles.hpp"
#include "../../../db/entities.hpp"

#include "../../../entities/conv.hpp"
#include "../../../consts.hpp"

#include "../../../libs/printing/printing.hpp"

#include <fstream>

using namespace gpkih;

int csv::exportDB(std::string_view outDir, std::vector<std::string> &args){
	auto profiles = db::profiles::get();
	
	std::ofstream profileFile(std::string{outDir} + SLASH + "gpkih_profiles.csv");
	std::ofstream entityFile(std::string{outDir} + SLASH + "gpkih_entities.csv");
	
	entityFile << "profile_id,serial,common_name,type,country,state,location,organisation,email,keypath,csrpath,crtpath,status,creation_date,expiration_date" << EOL;
	profileFile << "profile_id,name,source" << EOL;

	if(!profileFile.is_open() || !entityFile.is_open()){
		PERROR("Couldn't create required files\n");
		return GPKIH_FAIL;
	}

	for(const auto &[profileName, profile] : *profiles){
		auto entities = EntityManager(profileName);
		profileFile << profile.meta.id << ','  << profileName << ',' << profile.source << EOL;
		for(const auto &[cn, entity] : *entities.retrieve()){
			entityFile << profile.meta.id 
			<< ',' 
			<< entity.meta.serial 
			<< ',' 
			<< cn 
			<< ',' 
			<< entity::conversion::toString(entity.meta.type) 
			<< ',' 
			<< entity.subject.country 
			<< ',' 
			<< entity.subject.state 
			<< ',' 
			<< entity.subject.location 
			<< ',' 
			<< entity.subject.organisation 
			<< ',' 
			<< entity.subject.email 
			<< ',' 
			<< entity.keyPath
			<< ','
			<< entity.csrPath
			<< ','
			<< entity.crtPath
			<< ','
			<< entity::conversion::toString(entity.meta.status)
			<< ','
			<< fmt::format("{:%d-%m-%Y @ %H:%M}", entity.meta.creationDate)
			<< ','
			<< fmt::format("{:%d-%m-%Y @ %H:%M}", entity.meta.expirationDate)
			<< EOL; 
		}
	};

	profileFile.close();
	entityFile.close();

	return GPKIH_OK;
}