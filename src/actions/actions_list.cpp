#include "actions.hpp"


enum class PRINT_MODE {
  table,
  #define PRINT_TABLE PRINT_MODE::table
  csv
  #define PRINT_CSV PRINT_MODE::csv
};

using namespace gpki;

void print_profile(str profile, PROFILE_FIELDS &fields){
  std::stringstream out;
  fields == P_NAME && out << "Profile name: " << db::profiles::existing_profiles[profile].name << std::endl; 
  fields == P_SRC && out << "Profile source: " << db::profiles::existing_profiles[profile].source << std::endl;
}

void print_profile(str profile, std::vector<PROFILE_FIELDS> &fields, PRINT_MODE printmode = PRINT_TABLE){
  std::vector<str> pinfo;
  db::profiles::get_fields(profile,pinfo);
  if(printmode == PRINT_TABLE){
    /* Make the table presentation a bit better :) */
    if(fields.size()){
      for(int i = 0; i < fields.size() - 1 ; ++i){
        std::cout << pinfo[(uint8_t)fields[i]] << " | ";
      }
      std::cout << pinfo[(uint8_t)fields[fields.size()-1]]; 
    }
  }else{
    for(const auto &f : fields){
      std::cout << pinfo[(uint8_t)f] << ",";
    }
  }
}

void print_profile_entities(str profile, std::vector<ENTITY_FIELDS> &fields, PRINT_MODE printmode = PRINT_TABLE){
  std::vector<std::vector<str>> buff;
  db::profiles::get_entities(profile,buff); 
  for(auto entity : buff){
    if(printmode == PRINT_TABLE){
      for(auto field : fields){
        std::cout << entity[(uint8_t)field];
      }
    }
  }
}

int actions::list(subopts::list &params) {
  //for(auto pfields : params.pfields){
  //  std::cout << "Pfield -> " << (int)pfields << "\n";
  //}
  //for(auto efields : params.efields){
  //  std::cout << "Efield -> " << (int)efields << "\n";
  //}
  if(params.profiles.empty()){
    if(params.entities.empty()){
      // all profiles all entities      
      for(auto p : db::profiles::existing_profiles){
        print_profile(p.first,params.pfields);
        std::cout << "\n";
      }
    }else{
      // all profiles certain entities
    }
  }
  if(params.entities.empty()){
    for(auto p : params.profiles){
      print_profile(p,params.pfields,PRINT_CSV);
      print_profile_entities(p,params.efields,PRINT_CSV);
    }
    return 0;
    // certain profiles all entities
  }else{
    // certain profiles certain entities
  }
  return 0;
}
