#pragma once
#include "entities.hpp"
#include "../config/config_management.hpp"
#include "../db/entities.hpp"

namespace gpkih::subject
{
	extern int promptForSubject(std::string_view profileName, Subject &buffer, ProfileConfig &config, EntityManager &eman);
}