#include "../subj_utils.hpp"
#include "../../gpkih.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../libs/utils/utils.hpp"

static inline std::string badchars = "~`!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?¿\t\n\r ";
static inline std::string mail_badchars = "~`!#$%^&*()-_=+[{]}\\|;:'\",<>/?¿\t\n\r";

static inline size_t __get_and_set_prop(std::string_view promptMsg, const char *&defaultValue, const char *&st, size_t* stlen, std::string &badcharlist = badchars, size_t maxStLen = 254) {
  DEBUG(2,"__get_and_set_prop()");

  std::string input = PROMPT(promptMsg);

  if (input.empty()) {
    CALLOCATE(st, stlen, defaultValue);
    return 0;
  }

  if(gurgui::utils::str::remove_badchar(input, badcharlist) == true){
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

int subject::promptForSubject(std::string_view profileName, Subject &buffer, Subject &defaults, EntityManager &eman){
	DEBUG(2,"__prompt_for_subject()");

  std::string input{};

  // Set country if not set already
  if(buffer.country[0] == 0x00){
    PROMPT(fmt::format("Country Name (2 letter code) [{}]",defaults.country));
    if (!input.empty() && input.size() == 2) {
      memcpy(const_cast<char*>(buffer.country), input.c_str(), input.size());
    }else{
      memcpy(const_cast<char*>(buffer.country), defaults.country, 2);
    }
  }

  // Set state name if not set already
  if(buffer.state == nullptr){
    __get_and_set_prop(fmt::format("State or Province Name (full name) [{}]",defaults.state),defaults.state,buffer.state,reinterpret_cast<size_t*>(&buffer.meta.statelen));  
  }

  // Set location if not set already
  if(buffer.location == nullptr){
    __get_and_set_prop(fmt::format("Locality Name [{}]",defaults.location),defaults.location, buffer.location,reinterpret_cast<size_t*>(&buffer.meta.locationlen));
  }
  
  // Set organisation if not set already
  if(buffer.organisation == nullptr){
    __get_and_set_prop(fmt::format("Organisation Name [{}]",defaults.organisation),defaults.organisation, buffer.organisation,reinterpret_cast<size_t*>(&buffer.meta.organisationlen));
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
    }else if(gurgui::utils::str::has_badchar(input, badchars)){
      PWARN("Detected bad chars in common name, please avoid using special characters or spaces\n");
      continue;
    }

    // Got proper cn
    CALLOCATE(buffer.cn, reinterpret_cast<size_t*>(&buffer.meta.cnlen), input);
    input.assign("");
    break;
  }
  
  // Set email if not set already
  if(buffer.email == nullptr){
    __get_and_set_prop("Email Address",defaults.email, buffer.email, reinterpret_cast<size_t*>(&buffer.meta.emaillen), mail_badchars);  
  }

  return GPKIH_OK;
}

std::string subject::opensslOneliner(Subject &ref){
  std::basic_ostringstream<char> ss;
  
  strlen(ref.country) != 0       && ss << fmt::format("/C={}",ref.country);
  ref.state           != nullptr && ss << fmt::format("/ST={}",ref.state);
  ref.location        != nullptr && ss << fmt::format("/L={}", ref.location);
  ref.organisation    != nullptr && ss << fmt::format("/O={}",ref.organisation);
  ref.cn              != nullptr && ss << fmt::format("/CN={}",ref.cn);
  ref.email           != nullptr && ss << fmt::format("/emailAddress={}",ref.email);
  
  return ss.str();
}