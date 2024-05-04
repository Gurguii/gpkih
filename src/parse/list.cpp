#include "parser.hpp"
#include <sstream>

using namespace gpkih;
int parsers::list(std::vector<std::string> &opts) {
  ENTITY_FIELDS efields = E_ALL;
  PROFILE_FIELDS pfields = P_ALL;
  

  if (db::profiles::size() == 0) {
    PINFO("no profiles added yet\n");
    return GPKIH_OK;
  }
  
  if (opts.empty()) {
    return actions::list({}, pfields, efields);
  }

  std::string_view profile_name = opts[0];

  if(db::profiles::exists(profile_name) == false){
    PWARN("profile '{}' doesn't exist\n", profile_name);
    return GPKIH_FAIL;
  }

  opts.push_back("\0");
  auto emap = entity_fields_map();

  for (int i = 1; i < opts.size() - 1; ++i) {
    strview opt = opts[i];
    if (opt == "-ef" || opt == "--entity-fields") {
      if (opts[i + 1][0] == '-' || opts[i + 1] == "\0") {
        PWARN("entitiy fields is empty, could have been omitted\n");
      } else {
        efields = E_NONE;
        sstream ss(opts[++i]);
        str field;
        auto emap = entity_fields_map();
        while (getline(ss, field, ',')) {
          if (emap.find(field) != emap.end()) {
            efields = efields | emap[field];
          } else {
            PWARN("field '{}' doesn't exist\n", field);
          }
        }
      }
    } else if (opt == "-pf" || opt == "--profile-fields") {
      if (opts[i + 1][0] == '-' || opts[i + 1] == "\0") {
        PWARN("profile fields is empty, could have been omitted\n");
      } else {
        pfields = P_NONE;
        sstream ss(opts[++i]);
        str field;
        auto pmap = profile_fields_map();
        while (getline(ss, field, ',')) {
          if (pmap.find(field) != pmap.end()) {
            PROFILE_FIELDS f = pmap[field];
            pfields = pfields | f;
          } else {
            PWARN("Field '{}' doesn't exist\n", field);
          }
        }
      }
    } else {
      UNKNOWN_OPTION_MESSAGE(opt);
    }
  }
  return actions::list(profile_name, pfields, efields);
}
