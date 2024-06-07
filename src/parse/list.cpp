#include "parser.hpp"
#include <sstream>

// ./gpkih list <profile> <subopts>   | list entities from profile
// ./gpkih list <subopts>             |  list profiles

using namespace gpkih;
int parsers::list(std::vector<std::string> &opts) {
  if (db::profiles::size() == 0) {
    PINFO("no profiles added yet\n");
    return GPKIH_OK;
  }
  
  if (opts.empty()) {
    return actions::list_profiles(static_cast<uint16_t>(P_ALL));
  }
  
  opts.push_back("\0");

  std::string pname;
  uint16_t fields = static_cast<uint16_t>(0x7ff); // all fields except PATHS
  std::unordered_map<std::string,uint16_t> fmap;

  if(opts[0][0] != '-'){
    pname = opts[0];
    opts.erase(opts.begin());
    fmap = entityFieldsMap();
    if(db::profiles::exists(pname) == false){
      return GPKIH_FAIL;
    }
  }else{
    pname = {};
    fmap = profile_fields_map();
  }

  for(int i = 0; i < opts.size(); ++i){
    std::string_view opt = opts[i];
    if (opt == "-f" || opt == "--fields") {
      if (opts[i + 1][0] == '-'|| opts[i + 1] == "\0") {
        PINFO("profile fields is empty, could have been omitted\n");
      } else {
        fields = 0;
        sstream ss(opts[++i]);
        str field;
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

  return pname.empty() ? actions::list_profiles(fields) : actions::list_entities(pname, fields);
}