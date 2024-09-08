#pragma once

#include "../../profiles/structs.hpp"
#include "../../config/ProfileConfig.hpp"
#include "../../db/entities.hpp"

namespace gpkih::actions::add
{
	// TODO - add doxygen comments
	extern int add(Profile &profile, 
		ProfileConfig &config, 
		Entity &entity, 
		EntityManager &eman, 
		std::string_view days, 
		std::string_view keyAlgo, 
		std::string_view keySize, 
		bool autoanswer, 
		bool prompt
	);

	extern int addCA(Profile &profile, 
		ProfileConfig &config, 
		Entity &entity, 
		EntityManager &eman,
		std::string_view days, 
		std::string_view keyAlgo, 
		std::string_view keySize, 
		bool autoanswer, 
		bool prompt
	);
}