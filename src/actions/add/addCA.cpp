#include "add.hpp"

#include "../../entities/subj_utils.hpp"
#include "../../profiles/profiles.hpp"
#include "../../config/Config.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../consts.hpp"

using namespace gpkih;

static std::string __ca_build_command(Profile &profile, ConfigMap &pkiconf, Entity &entity, std::string_view days, std::string_view keyAlgo, std::string_view keySize){
  	DEBUG(2, "__ca_build_command()");
	
  	std::string command = std::move(fmt::format("openssl req -config {} -new -x509 -out {} -keyout {} -subj {} -set_serial {} -newkey {}:{} -noenc -days {}",
  	  profile::gopensslPath(profile),
  	  entity.crtPath,
  	  entity.keyPath,
  	  subject::opensslOneliner(entity.subject),
  	  entity.meta.serial,
  	  keyAlgo,
  	  keySize,
  	  days
  	));
  	
  	// Check for optional arguments based on configuration
  	if(Config::get("behaviour","autoanswer") == "yes"){
  	  command += " -batch"; // doesn't ask before signing certificate or updating database
  	}
	
  	return std::move(command);
};

int actions::add::addCA(Profile &profile, 
	ProfileConfig &config, 
	Entity &entity, 
	EntityManager &eman,
	std::string_view days, 
	std::string_view keyAlgo, 
	std::string_view keySize, 
	bool autoanswer, 
	bool prompt
){
	DEBUG(1,"actions::build_ca()");

  	if(autoanswer == false && profile.meta.caCreated){
  	  auto ans = PROMPT("Profile already has CA and\ncreating a new one will cause newly created certificates not to work with older ones , continue?", "[y/n]", true, LGREEN);
  	  if(ans != "y" && ans != "yes"){
  	    return GPKIH_FAIL;
  	  } 
  	}
	
  	ConfigMap &pkiconf = config.get(CFILE_PKI);
  	std::string command = std::move(__ca_build_command(profile,pkiconf,entity,days,keyAlgo,keySize));
  	
  	DEBUGF(3, "self-signed CA command - '{}'", command);
  	
  	if(system(command.c_str())){
  	  PERROR("command '{}' failed\n", command);
  	  return GPKIH_FAIL;
  	}
	
  	entity::incrementSerial(profile, entity);
  	
  	// Update profile modification date + caCreated status
  	profile.meta.lastModification = std::chrono::system_clock::now();
  	profile.meta.caCreated = 1;
	
  	return GPKIH_OK;
};