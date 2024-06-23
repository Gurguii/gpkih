#pragma once
#include "entities.hpp"
#include "../db/entities.hpp"

namespace gpkih::subject
{
	extern int promptForSubject(std::string_view profileName, Subject &buffer, Subject &defaultValues, EntityManager &eman);
	[[ nodiscard("Why the fuck do you ask for the oneliner if you're not using it?")]] 
	extern std::string opensslOneliner(Subject &ref);
}