#include "parser.hpp"

using namespace gpkih;
int parsers::list(std::vector<str> opts) {
  std::vector<str> profile_names;
  std::vector<str> entity_names;
  ENTITY_FIELDS efields = E_ALL;
  PROFILE_FIELDS pfields = P_ALL;

  if (opts.empty()) {
    return actions::list(profile_names, entity_names, pfields, efields);
  }
  if (opts[0][0] != '-') {
    sstream _profiles(opts[0]);
    str profile;
    while (getline(_profiles, profile, ',')) {
      if (!db::profiles::exists(profile)) {
        PERROR("profile '{}' doesn't exist, omitting...\n", profile);
        continue;
      }
      profile_names.push_back(profile);
    }
    opts.erase(opts.begin(), opts.begin() + 1);
  }
  opts.push_back("\0");
  auto emap = entity_fields_map();
  for (int i = 0; i < opts.size() - 1; ++i) {
    strview opt = opts[i];
    if (opt == "-e" || opt == "--entities") {
      strview value = opts[i + 1];
      if (value[0] != '-' && value != "\0") {
        // all entities
        str entity;
        sstream ss(str{opt[++i]});
        while (getline(ss, entity, ',')) {
          entity_names.push_back(entity);
        }
      }
    } else if (opt == "-ef" || opt == "--entity-fields") {
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
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  return actions::list(profile_names, entity_names, pfields, efields);
}
