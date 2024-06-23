#include "../conv.hpp"

using namespace gpkih;

std::string profile::conversion::toString(PROFILE_FIELDS pField){
	switch(pField){
	case PROFILE_FIELDS::name:
		return "name";
	case PROFILE_FIELDS::source:
		return "source";
	default:
		return "unknown";
	}
}