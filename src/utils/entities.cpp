#include "utils.hpp"

using namespace gpkih;

static inline std::string badchars = "~`!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?¿\t\n\r";
static inline std::string mail_badchars = "~`!#$%^&*()-_=+[{]}\\|;:'\",<>/?¿\t\n\r";

static inline size_t __get_and_set_prop(std::string &&promptMsg, char *&defaultValue, char *&st, size_t* stlen, std::string &badcharlist = badchars, size_t maxStLen = 254) {
  PDEBUG(2,"__get_and_set_prop()");

  std::string input = PROMPT(promptMsg);

  if (input.empty()) {
    CALLOCATE(st, stlen, defaultValue);
    return 0;
  }

  if(REMOVE_BADCHAR(input, badcharlist)){
    PWARN("badchars were removed\n");
  };

  if(input.size() <= maxStLen){
    CALLOCATE(st, stlen, input);
  }else{
    PWARN("Max length '{}' exceeded, not allocating memory\n", maxStLen);
  }

  return GPKIH_OK;
}

int utils::entities::promptForSubject(std::string_view profileName, Subject &buffer, ProfileConfig &config, EntityManager &eman){
	  PDEBUG(2,"__prompt_for_subject()");

  Subject defaults = config.default_subject();

  std::string input{};

  // Set country if not set already
  if(len(buffer.country) == 0){
    PROMPT(fmt::format("Country Name (2 letter code) [{}]",defaults.country));
    if (!input.empty() && input.size() == 2) {
      memcpy(buffer.country, input.c_str(), input.size() + 1);
    }else{
      memcpy(buffer.country, defaults.country, 2);
    }
  }

  // Set state name if not set already
  if(buffer.state == nullptr){
    __get_and_set_prop(fmt::format("State or Province Name (full name) [{}]",defaults.state),defaults.state,buffer.state,reinterpret_cast<size_t*>(&buffer.statelen));  
  }

  // Set location if not set already
  if(buffer.location == nullptr){
    __get_and_set_prop(fmt::format("Locality Name [{}]",defaults.location),defaults.location, buffer.location,reinterpret_cast<size_t*>(&buffer.locationlen));
  }
  
  // Set organisation if not set already
  if(buffer.organisation == nullptr){
    __get_and_set_prop(fmt::format("Organisation Name [{}]",defaults.organisation),defaults.organisation, buffer.organisation,reinterpret_cast<size_t*>(&buffer.organisationlen));
  }
  
  input.assign("");

  // *MANDATORY Set common name
  for(;;) {
    input = PROMPT("Common Name", false, RED);
    if (input.empty()) {
      // Common name can't be empty
      PWARN("common name can't be empty\n");
      continue;
    } else if (eman.exists(input)) {
      // Common name can't be duplicated
      PWARN("Entity with CN '{}' already exists in profile '{}'\n",
      input, profileName);
      continue;
    }else if(HAS_BADCHAR(input, badchars)) {
      continue;
    }

    // Got proper cn
    CALLOCATE(buffer.cn, reinterpret_cast<size_t*>(&buffer.cnlen), input);
    input.assign("");
    break;
  }
  
  // Set email if not set already
  if(buffer.email == nullptr){
    __get_and_set_prop("Email Address",defaults.email, buffer.email, reinterpret_cast<size_t*>(&buffer.emaillen), mail_badchars);  
  }

  return GPKIH_OK;
}

int utils::entities::setCAPaths(Profile &profile, Entity &e){
  e.keyPathLen = profile.sourcelen + 15; // 15 = /pki/ca/key.pem | /pki/ca/crt.pem
  e.crtPathLen = e.keyPathLen;
  
  e.keyPath = ALLOCATE(e.keyPathLen);
  e.crtPath = ALLOCATE(e.crtPathLen);

  if(e.keyPath == NULL || e.crtPath == NULL){
    PERROR("smth is null\n");
    return GPKIH_FAIL;
  }

  if(e.keyPathLen != snprintf(e.keyPath,e.keyPathLen+1,"%s%cpki%cca%ckey.pem",profile.source,SLASH,SLASH,SLASH)
    ||
     e.crtPathLen != snprintf(e.crtPath,e.crtPathLen+1,"%s%cpki%cca%ccrt.pem",profile.source,SLASH,SLASH,SLASH)){
    PERROR("snprintf() returned different than len\nkey:{}:%lu crt:{}:%lu\n",e.keyPath, e.keyPathLen, e.crtPath, e.crtPathLen);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}

int utils::entities::setPaths(Profile &profile, Entity &e){
  e.keyPathLen = profile.sourcelen + 18 + e.subject.cnlen; // /pki/keys/-key.pem 
  e.crtPathLen = profile.sourcelen + 19 + e.subject.cnlen; // /pki/certs/-crt.pem
  e.csrPathLen = profile.sourcelen + 18 + e.subject.cnlen; // /pki/reqs/-csr.pem
  
  e.keyPath = ALLOCATE(e.keyPathLen);
  e.csrPath = ALLOCATE(e.csrPathLen);
  e.crtPath = ALLOCATE(e.crtPathLen);

  if(e.keyPath == NULL || e.csrPath == NULL || e.crtPath == NULL){
    PERROR("SMTH IS NULL\n");
    return GPKIH_FAIL;
  }

  if(e.keyPathLen != snprintf(e.keyPath, e.keyPathLen+1,"%s%cpki%ckeys%c%s-key.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)
    ||
     e.csrPathLen != snprintf(e.csrPath, e.csrPathLen+1, "%s%cpki%creqs%c%s-csr.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)
    ||
     e.crtPathLen != snprintf(e.crtPath, e.crtPathLen+1, "%s%cpki%ccerts%c%s-crt.pem",profile.source,SLASH,SLASH,SLASH,e.subject.cn)){
    PERROR("snprintf() returned different than len\nkey:{} csr:{} crt:{}\n",e.keyPath, e.csrPath, e.crtPath);
    return GPKIH_FAIL;
  }

  return GPKIH_OK;
}