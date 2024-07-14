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

const profile::conversion::serializedProfile profile::conversion::serialize(Profile &p){
	int bufferSize = 31 + p.meta.nameLen + p.meta.sourceLen;
	std::byte *buffer = (std::byte*)calloc(bufferSize, sizeof(std::byte));
	
	if(buffer == nullptr){
		return {nullptr, 0};
	}	

	memcpy(buffer, &p.meta, sizeof(Profile::meta));
	int pos = sizeof(Profile::meta)+1;
	memcpy(buffer+pos, &p.name, p.meta.nameLen);
	pos += p.meta.nameLen + 1;
	memcpy(buffer+pos, &p.source, p.meta.sourceLen);

	return profile::conversion::serializedProfile{buffer, bufferSize};
}