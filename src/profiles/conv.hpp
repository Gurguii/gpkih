#pragma once
#include "enums.hpp"
#include "structs.hpp"
#include <string>

namespace gpkih::profile::conversion
{
	extern std::string toString(PROFILE_FIELDS pField);
	
	struct serializedProfile{
		std::byte *data;
		int size;
		~serializedProfile(){
			free(data);
			data = nullptr;
		}
	};
	[[nodiscard("returned pointer must be freed() manually")]] 
	extern const serializedProfile serialize(Profile &ref);
	
}