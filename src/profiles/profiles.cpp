#include "profiles.hpp"
#include "enums.hpp"

#ifdef _WIN32
constexpr char SLASH = '\\';
#else
constexpr char SLASH = '/';
#endif

using namespace gpkih;

std::string profile::toString(PROFILE_FIELDS pField){
	switch(pField){
	case PROFILE_FIELDS::name:
		return "name";
	case PROFILE_FIELDS::source:
		return "source";
	default:
		return "unknown";
	}
}

std::string profile::caCertificatePath(Profile &ref){
	return std::string{ref.source} + SLASH + "pki" + SLASH + "ca" + SLASH + "crt.pem";
};

std::string profile::gopensslPath(Profile &ref){
	return std::string{ref.source} + SLASH + "gopenssl.conf";
}

std::string profile::keyDir(Profile &ref){
	return std::string{ref.source} + SLASH + "pki" + SLASH + "keys" + SLASH;
}

std::string profile::reqDir(Profile &ref){
	return std::string{ref.source} + SLASH + "pki" + SLASH + "reqs" + SLASH;
}

std::string profile::crtDir(Profile &ref){
	return std::string{ref.source} + SLASH + "pki" + SLASH + "certs" + SLASH;
}

std::string profile::crlDir(Profile &ref){
	return std::string{ref.source} + SLASH + "pki" + SLASH + "reqs";
}