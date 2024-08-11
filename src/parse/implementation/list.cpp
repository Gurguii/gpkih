#include "../parser.hpp"
#include "../../profiles/enums.hpp"

using namespace gpkih;

static inline std::unordered_map<std::string, uint16_t>
entityFieldsMap() {
  return {
    {"cdate", static_cast<uint16_t>(E_CREATION_DATE)},
    {"cn", static_cast<uint16_t>(ENTITY_FIELDS::subject_cn)},
    {"type", static_cast<uint16_t>(ENTITY_FIELDS::type)},
    {"serial", static_cast<uint16_t>(ENTITY_FIELDS::serial)},
    {"country", static_cast<uint16_t>(ENTITY_FIELDS::subject_country)},
    {"state", static_cast<uint16_t>(ENTITY_FIELDS::subject_state)},
    {"location", static_cast<uint16_t>(ENTITY_FIELDS::subject_location)},
    {"org", static_cast<uint16_t>(ENTITY_FIELDS::subject_organisation)},
    {"mail", static_cast<uint16_t>(ENTITY_FIELDS::subject_email)},
    {"key", static_cast<uint16_t>(ENTITY_FIELDS::keyPath)},
    {"req", static_cast<uint16_t>(ENTITY_FIELDS::req_path)},
    {"crt", static_cast<uint16_t>(ENTITY_FIELDS::cert_path)},
    {"status", static_cast<uint16_t>(ENTITY_FIELDS::status)},
    {"edate", static_cast<uint16_t>(ENTITY_FIELDS::expirationDate)},
  };
}

static inline std::unordered_map<std::string, uint16_t>
profileFieldsMap() {
  return {
    {"id",static_cast<uint16_t>(PROFILE_FIELDS::id)},
    {"name", static_cast<uint16_t>(PROFILE_FIELDS::name)},
    {"source", static_cast<uint16_t>(PROFILE_FIELDS::source)},
    {"cdate", static_cast<uint16_t>(PROFILE_FIELDS::creationDate)},
    {"creationDate", static_cast<uint16_t>(PROFILE_FIELDS::creationDate)},
    {"lmod", static_cast<uint16_t>(PROFILE_FIELDS::lastModification)},
    {"lastModification", static_cast<uint16_t>(PROFILE_FIELDS::lastModification)},
    {"ca", static_cast<uint16_t>(PROFILE_FIELDS::caCreated)},
    {"sv", static_cast<uint16_t>(PROFILE_FIELDS::sv_count)},
    {"cl", static_cast<uint16_t>(PROFILE_FIELDS::cl_count)}
  };
}

// ./gpkih list <profile> <subopts>   | list entities from profile
// ./gpkih list <subopts>             |  list profiles
int parsers::list(std::vector<std::string> &opts) {
  DEBUG(1, "parsers::list()");

  if (db::profiles::size() == 0) {
    PINFO("no profiles added yet\n");
    return GPKIH_OK;
  }
  
  if (opts.empty()) {
    return actions::list(static_cast<uint16_t>(P_ALL));
  }
  
  opts.push_back("\0");

  std::string pname;

  uint16_t fields = static_cast<uint16_t>(0xfff); // all fields except PATHS to avoid big ass width by default
  std::unordered_map<std::string,uint16_t> fmap;

  if(opts[0][0] != '-'){
    pname = opts[0];
    opts.erase(opts.begin());
    fmap = entityFieldsMap();
    if(db::profiles::exists(pname) == false){
      PERROR("Profile '{}' doesn't exist\n", pname);
      return GPKIH_FAIL;
    }
  }else{
    pname = {};
    fmap = profileFieldsMap();
  }

  for(int i = 0; i < opts.size(); ++i){
    std::string_view opt = opts[i];
    if (opt == "-f" || opt == "--fields") {
      if (opts[i + 1][0] == '-'|| opts[i + 1] == "\0") {
        PINFO("profile fields is empty, could have been omitted\n");
      } else {
        fields = 0;
        std::stringstream ss(opts[++i]);
        std::string field;
        while (getline(ss, field, ',')) {
          if (fmap.find(field) != fmap.end()) {
            fields = fields | fmap[field];
          } else{
            PWARN("Field '{}' doesn't exist\n", field);
          }
        }
      }
    }else if(opt == "\0"){
      continue;
    }else{
      UNKNOWN_OPTION_MESSAGE(opt);
      continue;
    }
  }

  if(fields == 0){
    PINFO("No valid fields requested\n");
    PHINT("Try ./gpkih help list to get a list of available profile/entity fields\n");
    return GPKIH_OK;
  }

  return pname.empty() ? actions::list(fields) : actions::list(pname, fields);
}