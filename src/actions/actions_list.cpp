#include "actions.hpp"

enum class PRINT_MODE {
  table,
#define PRINT_TABLE PRINT_MODE::table
  csv
#define PRINT_CSV PRINT_MODE::csv
};

using namespace gpki;

void print_profile(str profile, PROFILE_FIELDS fields, COLOR color = CYAN ) {
  sstream label;
  auto &ref = db::profiles::existing_profiles;
  fields &P_NAME &&label << S_PLABEL("Profile name: ")
                         << S_PLABEL_V(ref[profile].name)
                         << "\n";
  fields &P_SRC &&label << S_PLABEL("Profile source: ")
                        << S_PLABEL_V(ref[profile].source)
                        << "\n";
  PRINT(label.str(),S_NONE);
}

void print_profile_entities(str profile, ENTITY_FIELDS &fields, COLOR color = YELLOW) {
  std::vector<Entity> entities;
  db::profiles::get_entities(profile, entities);
  sstream label;
  for(const auto &entity : entities){
    auto &subj = entity.subject;
    // Populate label and print 
    label << fmt::format(  fg(WHITE),"\n------------------------------------") << "\n";
    fields & E_COMMON   && label << S_ELABEL("Common name: ") << S_ELABEL_V(subj.cn) << "\n";
    fields & E_TYPE            && label << S_ELABEL("Type: ") << S_ELABEL_V(entity.type) << "\n";
    fields & E_COUNTRY   && label << S_ELABEL("Country: ") << S_ELABEL_V(subj.country) << "\n";
    fields & E_KEYPATH     && label << S_ELABEL("Key: ") <<  S_ELABEL_V(entity.key_path) << "\n";
    fields & E_REQPATH    && label << S_ELABEL("Request: ") <<  S_ELABEL_V(entity.req_path) << "\n";
    fields & E_CRTPATH     && label << S_ELABEL("Certificate: ") << S_ELABEL_V(entity.cert_path) << "\n";
    fields & E_ORG            && label << S_ELABEL("Organisation: ") << S_ELABEL_V(subj.organisation) << "\n";
    fields & E_LOCATION  && label << S_ELABEL("Location: ") << S_ELABEL_V(subj.location) << "\n";
    fields & E_MAIL           && label << S_ELABEL("Mail: ") << S_ELABEL_V(subj.email) << "\n";
    //label << fmt::format(  fg(WHITE),"------------------------------------") << "\n\n";
    PRINT(label.str(),S_NONE);
  }
}

int actions::list(subopts::list &params) {
  if (params.profiles.empty()) {
    if (params.entities.empty()) {
      /* OPTION 1 */
      // all profiles all entities
      for (auto p : db::profiles::existing_profiles) {
        print_profile(p.first, params.pfields);
        print_profile_entities(p.first, params.efields);
      }
    } else {
      /* OPTION 2 */
      // all profiles certain entities
    } 
  }
  if (params.entities.empty()) {
    /* OPTION 3 */
    // certain profiles all entities
    for (auto p : params.profiles) {
      print_profile(p, params.pfields);
      print_profile_entities(p, params.efields);
    }
    return 0;
  } else {
    /* OPTION 4 */
    // certain profiles certain entities

  }
  return 0;
}