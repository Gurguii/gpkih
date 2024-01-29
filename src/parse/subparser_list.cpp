#include "subparser.hpp"

using namespace gpki;
int subparsers::list(std::vector<std::string> opts) {
  subopts::list params;
  if (opts.size() == 0) {
    return actions::list(params);
  }
  if (opts[0][0] != '-') {
    sstream _profiles(opts[0]);
    str profile;
    while (getline(_profiles, profile, CSV_DELIMITER_c)) {
      params.profiles.push_back(profile);
    }
    opts.erase(opts.begin(), opts.begin() + 1);
  }
  opts.push_back("\0");
  for (int i = 0; i < opts.size() - 1; ++i) {
    std::string_view opt = opts[i];
    if (opt == "-cn" || opt == "--common-name") {
      std::string_view value = opts[i + 1];
      if (value[0] == '-' || value == "\0") {
        std::cout << "no common name given, defaulting to ALL\n";
      } else {
        str entity;
        sstream ss(str{opt[++i]});
        while (getline(ss, entity, CSV_DELIMITER_c)) {
          params.entities.push_back(entity);
        }
      }
    } else if (opt == "-ef" || opt == "--entity-fields") {
      if(opts[i+1][0] == '-' || opts[i+1] == "\0"){
        PWARN("entitiy fields is empty, could have been omitted\n");
      }else{
        params.efields = E_NONE;
        std::stringstream ss(opts[++i]);
        std::string field;
        auto emap = entity_fields_map();
        while (getline(ss, field, CSV_DELIMITER_c)) {
          if (emap.find(field) != emap.end()) {
            ENTITY_FIELDS f = emap[field];
            params.efields = params.efields | f;
          } else {
            PINFO("field '{}' doesn't exist",field);
          }
        }
      }
    } else if (opt == "-pf" || opt == "--profile-fields") {
      if(opts[i+1][0] == '-' || opts[i+1] == "\0"){
        PWARN("profile fields is empty, could have been omitted\n");
      }else{
        params.pfields = P_NONE;
        std::stringstream ss(opts[++i]);
        std::string field;
        auto pmap = profile_fields_map();
        while (getline(ss, field, CSV_DELIMITER_c)) {
          if (pmap.find(field) != pmap.end()) {
            PROFILE_FIELDS f = pmap[field];
            params.pfields = params.pfields | f;
          } else {
            std::cout << "Field '" << field << "' doesn't exist\n";
          }
        }
      }
    } else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  return actions::list(params);
}