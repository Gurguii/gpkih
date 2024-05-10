#include "actions.hpp"
#include "fmt/color.h"
#include <sstream>
#include <string>
#include <array>

using namespace gpkih;

static inline std::vector<PROFILE_FIELDS> pfield_arr{P_ID,P_NAME,P_SRC,P_CREATION_DATE,P_LAST_MODIFICATION,P_CA_CREATED,P_SV_COUNT,P_CL_COUNT};
static inline std::vector<ENTITY_FIELDS> efield_arr{E_SERIAL,E_COMMON,E_CREATION_DATE,E_TYPE,E_STATUS,E_COUNTRY,E_STATE,E_LOCATION,E_ORG,E_MAIL,E_KEYPATH,E_REQPATH,E_CRTPATH};

static inline std::string_view date_format;
static inline int date_width;

// @brief print profile headers
template <typename T> void __header_print(uint16_t fields, std::vector<const char*> &values,std::vector<int> &widths, STYLE style = S_NONE){
  PDEBUG(2, "__header_print(template)");

  if(values.size() != widths.size()){
    return;
  }  

  std::stringstream entry{};

  if(std::is_base_of_v<T,Profile>){
    for(int i = 0; i < values.size(); ++i){
      fields & pfield_arr[i] && entry << fmt::format("{:^" + std::to_string(widths[i]) + "}",values[i]);
    }
  }else if(std::is_base_of_v<T,Entity>){
    for(int i = 0; i < values.size(); ++i){
      fields & efield_arr[i] && entry << fmt::format("{:^" + std::to_string(widths[i]) + "}",values[i]);
    }
  }

  fmt::print(style, "{}\n",entry.str());
}

// @brief print profiles in table form
static inline void __table_print(PROFILE_FIELDS pfields, std::map<std::string_view,Profile>* profiles, std::vector<int> &widths, STYLE style = S_NONE){
  PDEBUG(2,"__table_print(Profile)");

  if(widths.size() != pfield_arr.size()){
    return;
  }
  size_t pos = 0;

  for(const auto &[name,p] : *profiles){
    std::stringstream entry{};
    // Add requested fields
    pfields & P_ID                && entry << fmt::format("{:^4}",p.id);
    pfields & P_NAME              && entry << fmt::format("{:^" + std::to_string(widths[1])  + "}",p.name);
    pfields & P_SRC               && entry << fmt::format("{:^" + std::to_string(widths[2])  + "}",p.source);
    pfields & P_CREATION_DATE     && entry << fmt::format("{:^" + std::to_string(date_width) + "}",fmt::format(date_format,p.creation_date));
    pfields & P_LAST_MODIFICATION && entry << fmt::format("{:^" + std::to_string(date_width) + "}",fmt::format(date_format,p.last_modification));
    pfields & P_CA_CREATED        && entry << fmt::format("{:^12}", (p.ca_created ? "yes" : "no"));
    pfields & P_SV_COUNT          && entry << fmt::format("{:^14}", p.sv_count);
    pfields & P_CL_COUNT          && entry << fmt::format("{:^14}", p.cl_count);
    // Print entry
    fmt::print(style, "{}\n", entry.str());
  }
}

// @brief prints Profile/Entities map in table form
// @param fields desired PROFILE_FIELDS or ENTITY_FIELDS to show, added via -f on `./gpkih list` action
template <typename T> static inline void __table_print(ENTITY_FIELDS fields, const std::map<std::string_view,T>* data, std::vector<int> &widths, STYLE style = S_NONE){
  PDEBUG(2,"__table_print(Entity)");
  for(const auto &[cn,e] : *data){
    const Subject &s = e.subject;
    std::stringstream entry{};

    std::string status;    
    switch(e.status){
    case ES_ACTIVE:
      status = fmt::format(fg(TB_GREEN) | EMPHASIS::bold, "active");
      break;
    case ES_REVOKED:
      status = fmt::format(fg(TB_RED) | EMPHASIS::bold, "revoked");
      break;
    case ES_MARKED:
      status = fmt::format(fg(TB_YELLOW) | EMPHASIS::bold, "marked");
      break;
    default:
      status = fmt::format(fg(TB_BLUE) | EMPHASIS::bold, "unknown");
      break;;
    }

    // Add requested ENTITY fields
    fields & E_SERIAL        && entry << fmt::format("{:^" + std::to_string(widths[0])  + "}", e.serial);
    fields & E_COMMON        && entry << fmt::format("{:^" + std::to_string(widths[1])  + "}", s.cn);
    fields & E_CREATION_DATE && entry << fmt::format("{:^" + std::to_string(widths[2])  + "}", fmt::format(date_format,e.creation_date));
    fields & E_TYPE          && entry << fmt::format("{:^" + std::to_string(widths[3])  + "}", to_str(e.type));
    fields & E_STATUS        && entry << fmt::format("{:^" + std::to_string(widths[4])  + "}", status);
    fields & E_COUNTRY       && entry << fmt::format("{:^" + std::to_string(widths[5])  + "}", s.country);
    fields & E_STATE         && entry << fmt::format("{:^" + std::to_string(widths[6])  + "}", s.state);
    fields & E_LOCATION      && entry << fmt::format("{:^" + std::to_string(widths[7])  + "}", s.location);
    fields & E_ORG           && entry << fmt::format("{:^" + std::to_string(widths[8])  + "}", s.organisation);
    fields & E_MAIL          && entry << fmt::format("{:^" + std::to_string(widths[9])  + "}", s.email);
    fields & E_KEYPATH       && entry << fmt::format("{:^" + std::to_string(widths[10]) + "}", e.key_path);
    fields & E_REQPATH       && entry << fmt::format("{:^" + std::to_string(widths[11]) + "}", e.crt_path);
    fields & E_CRTPATH       && entry << fmt::format("{:^" + std::to_string(widths[12]) + "}", e.csr_path);
    
    // Print entry
    fmt::print(style, "{}\n",entry.str());
  }
}

int __set_date(){
  PDEBUG(2, "__set_date()");
  date_format = Config::get("formatting","date_format");
  if(date_format.empty()){
    return GPKIH_FAIL;
  }
  date_width = std::max(static_cast<size_t>(21),fmt::format(date_format,std::chrono::system_clock::now()).size()+2);
  return GPKIH_OK;
}

int actions::list_profiles(uint16_t fields){
  PDEBUG(1, "actions::list_profiles()");

  if(__set_date() == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  auto profiles = db::profiles::get();
  std::vector<const char*> headers{"id","name","source","creation_date","last_modification","ca_created","server_count","client_count"};
  std::vector<int> widths{4,6,8,date_width,date_width,12,14,14};

  for(const auto &kv : *profiles){
    const Profile &p = kv.second;
    widths[1] = std::max(widths[1],static_cast<int>(p.namelen));
    widths[2] = std::max(widths[2],static_cast<int>(p.sourcelen));
  }

  // Add padding
  widths[1]+=2;
  widths[2]+=2;
  // Print headers
  __header_print<Profile>(static_cast<uint16_t>(fields), headers, widths, fg(T_GREEN) | EMPHASIS::bold);
  // Print profiles
  __table_print(static_cast<PROFILE_FIELDS>(fields), profiles, widths);
  return GPKIH_OK;
};

int actions::list_entities(std::string_view profile_name,uint16_t fields){
  PDEBUG(1, "actions::list_entities()");

  if(__set_date() == GPKIH_FAIL){
    return GPKIH_FAIL;
  }

  EntityManager eman(profile_name);
  if(eman.empty()){
    PINFO("profile '{}' has no entities yet\n", profile_name);
    return GPKIH_OK;
  }

  auto entity_list = eman.retrieve();
  std::vector<const char *> headers{"serial","common_name","creation_date","type","status","country","state","location","organisation","email","key_path","csr_path","crt_path"};
  std::vector<int> widths{8,13,date_width,8,6,9,7,10,14,7,10,10,10};

  // Set appropiate widths
  for(const auto &kv : *entity_list){
    const Entity &e = kv.second;
    const Subject &s = e.subject;
    widths[1] = std::max(widths[1], static_cast<int>(s.cnlen)) + 2;
    widths[6] = std::max(widths[6], static_cast<int>(s.statelen)) + 2;
    widths[7] = std::max(widths[7], static_cast<int>(s.locationlen)) + 2;
    widths[8] = std::max(widths[8], static_cast<int>(s.organisationlen)) + 2;
    widths[9] = std::max(widths[9], static_cast<int>(s.emaillen)) + 2;
    widths[10] = std::max(widths[10], static_cast<int>(e.key_path_len)) + 2;
    widths[11] = std::max(widths[11], static_cast<int>(e.csr_path_len)) + 2;
    widths[12] = std::max(widths[12], static_cast<int>(e.crt_path_len)) + 2;
  }
  // Print headers
  __header_print<Entity>(static_cast<uint16_t>(fields), headers, widths, fg(T_GREEN) | EMPHASIS::bold);
  // Print entities
  __table_print(static_cast<ENTITY_FIELDS>(fields), entity_list, widths); 
  return GPKIH_OK;
};