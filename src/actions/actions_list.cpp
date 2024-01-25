#include "actions.hpp"


enum class PRINT_MODE {
  table,
  #define PRINT_TABLE PRINT_MODE::table
  csv
  #define PRINT_CSV PRINT_MODE::csv
};

using namespace gpki;

void print_profile(str profile, uint8_t &fields){
  std::stringstream label;
  fields & P_NAME && label << "Profile name: " << db::profiles::existing_profiles[profile].name << std::endl; 
  fields & P_SRC && label << "Profile source: " << db::profiles::existing_profiles[profile].source << std::endl;
  std::cout << label.str() << "\n";
}

void print_profile_entities(str profile, ENTITY_FIELDS &fields){

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
      /* OPTION 1 */
      // all profiles all entities      
      for(auto p : db::profiles::existing_profiles){
        std::cout << p.first << "\n";
        print_profile(p.first,params.pfields);
        std::cout << "\n";
      }
    }else{
      /* OPTION 2 */
      // all profiles certain entities
    }
  }
  if(params.entities.empty()){
    for(auto p : params.profiles){
      print_profile(p,params.pfields);
    }
    return 0;
    /* OPTION 3 */
    // certain profiles all entities
  }else{
    /* OPTION 4 */
    // certain profiles certain entities
  }
  return 0;
}
