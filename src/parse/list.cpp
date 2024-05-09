#include "parser.hpp"
#include <sstream>

// ./gpkih list <subopts>
// ./gpkih list <profile> <subopts>

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
  
  opts.push_back("\0");

  if (opts[0][0] == '-'){
    /* Not a profile name, parse specific 
       subopts and print all profiles */
    for(int i = 0; i < opts.size(); ++i){
      std::string_view opt = opts[i];
      if (opt == "-pf" || opt == "--profile-fields") {
        if (opts[i + 1][0] == '-'|| opts[i + 1] == "\0") {
          PINFO("profile fields is empty, could have been omitted\n");
        } else {
          pfields = P_NONE;
          sstream ss(opts[++i]);
          str field;
          auto pmap = profile_fields_map();
          while (getline(ss, field, ',')) {
            if (pmap.find(field) != pmap.end()) {
              pfields = pfields | pmap[field];
            } else {
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
    return actions::list({},pfields,efields);
  }else{
    std::string_view profile_name = opts[0];
    /* Got a profile name, check, parse, and print entities */
    if(db::profiles::exists(profile_name) == false){
      PWARN("Profile '{}' doesn't exist\n", profile_name);
      return GPKIH_FAIL;
    }
    
    for(int i = 1; i < opts.size(); ++i){
      std::string_view opt = opts[i];
      if (opt == "-ef" || opt == "--entity-fields") {
        if (opts[i + 1][0] == '-' || opts[i + 1] == "\0") {
          PINFO("entitiy fields is empty, could have been omitted\n");
        } else {
          efields = E_NONE;
          sstream ss(opts[++i]);
          str field;
          auto emap = entity_fields_map();
          while (getline(ss, field, ',')) {
            if (emap.find(field) != emap.end()) {
              efields = efields | emap[field];
            } else {
              PWARN("Field '{}' doesn't exist\n", field);
            }
          }
        }
      }else if(opt == "\0"){
        continue;
      }else{
        UNKNOWN_OPTION_MESSAGE(opt);
      }
    }
    return actions::list(profile_name, pfields, efields);
  }
}