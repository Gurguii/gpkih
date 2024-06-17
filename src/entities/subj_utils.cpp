#include "subj_utils.hpp"
#include "../gpkih.hpp"
#include "../printing/printing.hpp"
#include "../utils/utils.hpp"

static inline std::string badchars = "~`!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?¿\t\n\r ";
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

using namespace gpkih;

int subject::promptForSubject(std::string_view profileName, Subject &buffer, ProfileConfig &config, EntityManager &eman){
	PDEBUG(2,"__prompt_for_subject()");

  Subject defaults = config.default_subject();

  std::string input{};

  // Set country if not set already
  if(buffer.country[0] == 0x00){
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