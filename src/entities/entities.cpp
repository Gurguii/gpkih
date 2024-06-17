#include "entities.hpp"
#include "../printing/printing.hpp"
#include "../gpkih.hpp"

using namespace gpkih;

std::string entity::toString(ENTITY_TYPE type) {
  switch(static_cast<uint8_t>(type)){
  	case ET_CA:
  	  return "ca";
  	case ET_CL:
  	  return "cl";
  	case ET_SV:
  	  return "sv";
  	default:
  	  return "none";
  }
};

std::string entity::toString(ENTITY_STATUS status){
  switch(status){
    case ES_ACTIVE:
      return "active";
    case ES_REVOKED:
      return "revoked";
    case ES_MARKED:
      return "marked";
    default:
      return "unknown";
  }
}

int entity::setCAPaths(Profile &profile, Entity &e){
  e.keyPathLen = profile.sourcelen + 15; // 15 = /pki/ca/key.pem | /pki/ca/crt.pem
  e.crtPathLen = e.keyPathLen;
  
  e.keyPath = ALLOCATE(e.keyPathLen);
  e.crtPath = ALLOCATE(e.crtPathLen);

  if(e.keyPath == nullptr || e.crtPath == nullptr){
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

int entity::setPaths(Profile &profile, Entity &e){
  e.keyPathLen = profile.sourcelen + 18 + e.subject.cnlen; // /pki/keys/-key.pem 
  e.crtPathLen = profile.sourcelen + 19 + e.subject.cnlen; // /pki/certs/-crt.pem
  e.csrPathLen = profile.sourcelen + 18 + e.subject.cnlen; // /pki/reqs/-csr.pem
  
  e.keyPath = ALLOCATE(e.keyPathLen);
  e.csrPath = ALLOCATE(e.csrPathLen);
  e.crtPath = ALLOCATE(e.crtPathLen);

  if(e.keyPath == nullptr || e.csrPath == nullptr || e.crtPath == nullptr){
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

int entity::incrementSerial(Profile &profile, Entity &entity){
  PDEBUG(2,"__increment_serial()");
  std::string serialPath = fmt::format("{}{}pki{}serial{}serial", profile.source, SLASH, SLASH, SLASH);

  std::ifstream rfile(serialPath);
  
  if(!rfile.is_open()){
    PERROR("Couldn't open serial file '{}' to update serial\n", serialPath);
    return GPKIH_FAIL;
  }

  std::string nserial{};
  rfile >> nserial;

  nserial = fmt::format("{:x}",static_cast<decltype(Entity::serial)>(std::stoull(nserial,0,16) + 1));

  rfile.close();
  
  std::ofstream wfile(serialPath);
  if(!wfile.is_open()){
    return GPKIH_FAIL;
  }

  if(nserial.size() == 1){
    wfile << "0";
  }

  wfile << nserial;
  
  wfile.close();

  return GPKIH_OK;
}

std::string entity::opensslOneliner(Subject &ref){
  std::ostringstream ss;
  
  strlen(ref.country) != 0 && ss << fmt::format("/C={}",ref.country);
  ref.state != nullptr && ss << fmt::format("/ST={}",ref.state);
  ref.location != nullptr && ss << fmt::format("/L={}", ref.location);
  ref.organisation != nullptr && ss << fmt::format("/O={}",ref.organisation);
  ref.cn != nullptr && ss << fmt::format("/CN={}",ref.cn);
  ref.email != nullptr && ss << fmt::format("/emailAddress={}",ref.email);
  
  return ss.str();
}

int entity::loadSerial(Profile &profile, Entity &entity){
  PDEBUG(2,"__load_serial()");
  std::string serialPath = fmt::format("{}{}pki{}serial{}serial", profile.source, SLASH, SLASH, SLASH);

  if(!std::filesystem::exists(serialPath)){
    PERROR("serial file for profile '{}' not found - '{}'\n", profile.name, serialPath);
    return GPKIH_FAIL;
  }

  std::ifstream file(serialPath);

  if(!file.is_open()){
    PERROR("couldn't open file '{}'\n",serialPath);
    return GPKIH_FAIL;
  }

  std::string serial{};
  file >> serial;
  file.close();

  try{
    entity.serial = std::stoull(serial,nullptr,16);  
  }catch(const std::invalid_argument &err){
    PINFO("Loading serial failed, generating random serial...\n");
    entity.serial = rand();
  }

  return GPKIH_OK;
}
/*
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
      PWARN("Common name can't be empty\n");
      continue;
    } else if (eman.exists(input)) {
      // Common name can't be duplicated
      PWARN("Entity with CN '{}' already exists in profile '{}'\n",
      input, profileName);
      continue;
    }else if(HAS_BADCHAR(input, badchars)){
      PWARN("Detected bad chars in common name, please avoid using special characters or spaces\n");
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
*/