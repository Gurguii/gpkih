#include "subparser.hpp"

using namespace gpki;
int subparsers::list(std::vector<std::string> opts) {
  opts.push_back("\0"); // avoid std::out_of_range when accesing like [++i]
  subopts::list params;
  for (int i = 0; i < opts.size() - 1; ++i) {
    std::string_view opt = opts[i];
    if (opt == "-p" || opt == "--profile") {
      std::string_view value = opts[i + 1];
      if (value[0] == '-' || value == "\0") {
        std::cout << "no profile given, defaulting to ALL\n";
      } else {
        str profile;
        sstream ss(str{opt[++i]});
        while (getline(ss, profile, CSV_DELIMITER_c)) {
          params.profile.push_back(profile);
        }
      }
    } else if (opt == "-cn" || opt == "--common-name") {
      std::string_view value = opts[i + 1];
      if (value[0] == '-' || value == "\0") {
        std::cout << "no common name given, defaulting to ALL\n";
      } else {
        str entity;
        sstream ss(str{opt[++i]});
        while (getline(ss, entity, CSV_DELIMITER_c)) {
          params.common_name.push_back(entity);
        }
      }
    } else if (opt == "-ef" || opt == "--entity-fields") {
      std::stringstream ss(opts[++i]);
      std::string field;
      auto emap = entity_fields_map();
      while (getline(ss, field, CSV_DELIMITER_c)) {
        if (emap.find(field) != emap.end()) {
          params.efields.push_back(field);
        }
      }
    } else if (opt == "-pf" || opt == "--profile-fields") {
      std::stringstream ss(opts[++i]);
      std::string field;
      auto pmap = profile_fields_map();
      while (getline(ss, field, FIELD_DELIMITER_c)) {
        if (pmap.find(field) != pmap.end()) {
          params.pfields.push_back(field);
        }
      }
    } else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  return actions::list(params);
}
