#include "actions.hpp"
#include "fmt/color.h"
#include <sstream>
/* UNUSED, left to see if i ever get to develop something around this idea */
enum class PRINT_MODE {
  table,
#define PRINT_TABLE PRINT_MODE::table
  csv,
#define PRINT_CSV PRINT_MODE::csv
};

using namespace gpkih;

int actions::list(std::string_view profile_name, PROFILE_FIELDS pfields, ENTITY_FIELDS efields) {
  // Check if existing_profiles is empty
  auto existing_profiles = db::profiles::get();

  if (profile_name.empty()) {
    /* OPTION 1 - print all profiles */
    /* id:4 name:6 source:8 creation_date:20 last_modification:20 ca:6 server_certificates:4 client_certificates:4 */
    
    std::vector<int> hw{4,6}; // name - source header widths - the rest is size-fixed
    for(const auto &kv : *existing_profiles){
      const Profile &p = kv.second;
      hw[0] = std::max(hw[0],static_cast<int>(p.namelen));
      hw[1] = std::max(hw[1],static_cast<int>(p.sourcelen));
    }

    std::stringstream ss{};
    ss << "{:^4}" << "{:^" << hw[0] << "}" << "{:^" << hw[1] << "}" << "{:^20}{:^20}{:^12}{:^21}{:^21}";

    // Print headers
    fmt::print(S_SUCCESS,ss.str(),"id","name","source","creation_date","last_modification","ca_created","server_certificates","client_certificates");
    fmt::print("\n");

    for(const auto &kv : *existing_profiles){
      const Profile &p = kv.second;
      fmt::print(ss.str(),p.id,p.name,p.source,fmt::format("{:%d-%m-%Y @ %H:%M}",p.creation_date),fmt::format("{:%d-%m-%Y @ %H:%M}",p.last_modification),(p.ca_created ? "yes" : "no"),p.sv_count,p.cl_count);
      fmt::print("\n");
    }

    return GPKIH_OK;
  }else{
    /* OPTION 2 - print profile entities */
    //fmter.print_headers("serial","common_name","type","country","state","organisation","email","key_path","csr_path","crt_path","creation_date");
    EntityManager eman(profile_name);
    
    if(eman.size() == 0){
      PINFO("profile '{}' has no entities yet\n", profile_name);
      return GPKIH_OK;
    }
    auto entity_list = *eman.retrieve();
    //fmter.print_headers("serial","common_name","type","country","state","organisation","email","key","csr","crt");
    // serial = width 10
    // type = width 10
    // country = width 5
    // creation_date = 15
    // 02/02/2023 -> 10 +2 = 12
    int cnlen = 13,countrylen = 9,stlen = 7, loclen = 10, orglen = 14,maillen = 7;
    // keylen = 5,csrlen = 5,crtlen = 5;
    for(const auto &kv : entity_list){
      const Entity &e = kv.second;
      const Subject &s = e.subject;
      cnlen = std::max(cnlen, static_cast<int>(s.cnlen) + 2 );
      stlen = std::max(stlen, static_cast<int>(s.statelen) + 2);
      orglen = std::max(orglen, static_cast<int>(s.organisationlen) + 2);
      maillen = std::max(maillen, static_cast<int>(s.emaillen) + 2);
      //keylen = std::max(keylen, static_cast<int>(e.key_path_len) + 2);
      //csrlen = std::max(csrlen, static_cast<int>(e.csr_path_len) + 2);
      //crtlen = std::max(crtlen, static_cast<int>(e.crt_path_len) + 2);
    }
    std::stringstream ss{};
    // serial, cn, creation_date, type, country ...
    ss << "{:^8}" << "{:^" << cnlen << "}" << "{:^20}" << "{:^8}" << "{:^9}" << "{:^9}" << "{:^" << stlen << "}" << "{:^" << loclen << "}" << "{:^" << orglen << "}" << "{:^" << maillen << "}";

    // TODO - think about this:
    // if ( add_paths == true) { ss << "{:^" << keylen << "}" << "{:^" << csrlen <<"}" << "{:^" << crtlen << "}"; }
    
    /* Print headers with proper column width */
    fmt::print(S_SUCCESS, ss.str(), "serial","common_name","creation_date","type","status","country","state","location","organisation","email");
    fmt::print("\n");

    /* Start printing entities per line */
    for(const auto &kv : entity_list){
      const Entity &e = kv.second;
      const Subject &s = e.subject;
      std::string status;
      
      switch(e.status){
      case ES_ACTIVE:
        status = fmt::format(fg(fmt::terminal_color::bright_green) | EMPHASIS::bold, "active");
        break;
      case ES_REVOKED:
        status = fmt::format(fg(fmt::terminal_color::bright_red) | EMPHASIS::bold, "revoked");
        break;
      case ES_MARKED:
        status = fmt::format(fg(fmt::terminal_color::bright_yellow) | EMPHASIS::bold, "marked");
        break;
      default:
        status = fmt::format(fg(fmt::terminal_color::bright_blue) | EMPHASIS::bold, "unknown");
        break;;
      }

      fmt::print(ss.str(), e.serial, s.cn, fmt::format("{:%d-%m-%Y @ %H:%M}",e.creation_date), str_conversion(e.type), fmt::format("{:^22}",status), s.country, s.state, s.location, s.organisation, s.email, e.key_path, e.csr_path, e.crt_path);
      fmt::print("\n");
    }
  }
  // certain profiles all entities
  return GPKIH_OK;
}