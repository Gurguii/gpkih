#include "AGencrl.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../profiles/profiles.hpp"
#include "../../db/profiles.hpp"
#include "../../gpkih.hpp"

using namespace gpkih;

static int generateNewCRL(Profile &profile){
  DEBUGF(3, "generateNewCRL({})", profile.name);
  
  if(profile.meta.caCreated == false){
    PWARN("Cannot generate a crl if no CA has been created\n");
    PHINT("Try './gpkih build <profile> ca -cn myCA' to create a certificate authority\n");
    return GPKIH_OK;
  }

  std::string crlPath = std::string{profile::crlDir(profile)}+SLASH+"current.pem";

  std::string command =
      fmt::format("openssl ca -config {} -gencrl -out {}",
                  profile::gopensslPath(profile), crlPath);

  if (system(command.c_str())) {
    PERROR("openssl gencrl command failed - '{}'\n", command);
    return GPKIH_FAIL;
  }

  PSUCCESS("Generated CRL - {}\n", crlPath);
  ADD_LOG(LL_INFO,fmt::format("profile:{} action:gencrl",profile.name));
  
  return GPKIH_OK;
}

int AGencrl::exec(std::vector<std::string> &args) const {

	/* BEG - Parse arguments */
	 DEBUG(1, "parsers::gencrl()");

  	// ./gpki gencrl <profile> [subopts]
  	if (args.empty()) {
  	  PERROR("profile must be given\n");
  	  PINFO("try gpki help gencrl\n");
  	  return -1;
  	}

  	std::string_view profilename = args[0];

  	Profile profile;
  	
    if (db::profiles::load(profilename, profile)) {
  	  return GPKIH_FAIL;
  	}
  /* END - Parse arguments */
	
	return generateNewCRL(profile);
};