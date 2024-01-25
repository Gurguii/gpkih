#include "subparser.hpp"

using namespace gpki;
int subparsers::list(std::vector<std::string> opts) {
  subopts::list params;
  if(opts.size() == 0){
    return actions::list(params);
  }
  if(opts[0][0] != '-'){
    sstream _profiles(opts[0]);
    str profile;
    while(getline(_profiles,profile,CSV_DELIMITER_c)){
      params.profiles.push_back(profile);
    }
    opts.erase(opts.begin(),opts.begin()+1);
  }
  opts.push_back("\0");
  for (int i = 0; i < opts.size() - 1 ; ++i) {
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
      params.efields = E_NONE;
      std::stringstream ss(opts[++i]);
      std::string field;
      auto emap = entity_fields_map();
      while (getline(ss, field, CSV_DELIMITER_c)) {
        if (emap.find(field) != emap.end()) {
          params.efields |= emap[field];
        }else{
          std::cout << "Field '" << field << "' doesn't exist\n";
        }
      }
    } else if (opt == "-pf" || opt == "--profile-fields") {
      std::cout << "before: " << (int)params.pfields << "\n";
      params.pfields = P_NONE;
      std::stringstream ss(opts[++i]);
      std::string field;
      auto pmap = profile_fields_map();
      while (getline(ss, field, CSV_DELIMITER_c)) {
        if (pmap.find(field) != pmap.end()) {
          params.pfields |= pmap[field];
        }else{
          std::cout << "Field '" << field << "' doesn't exist\n";
        }
      }
      std::cout << "after: " << (int)params.pfields << "\n";
    } else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  return actions::list(params);
}
