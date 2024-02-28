#pragma once
#include "gpki.hpp" // typename aliases + fmtlib
#include <unordered_map>

namespace gpkih {
enum class PROFILE_FIELDS : uint16_t {
  all = 6,
#define P_ALL PROFILE_FIELDS::all
  none = 0,
#define P_NONE PROFILE_FIELDS::none
  name = 2,
#define P_NAME PROFILE_FIELDS::name
  source = 4
#define P_SRC PROFILE_FIELDS::source
};
/* PROFILE OPERATORS */
static PROFILE_FIELDS operator|(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<PROFILE_FIELDS>((ui16)lo | (ui16)ro);
};
static bool operator&(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<bool>((ui16)lo & (ui16)ro);
}
/* PROFILE STRING -> PROFILE_FIELD MAP*/
static inline std::unordered_map<std::string, PROFILE_FIELDS>
profile_fields_map() {
  return {
      {"name", PROFILE_FIELDS::name},
      {"source", PROFILE_FIELDS::source},
  };
}
enum class ENTITY_TYPE {
  none = 0,
#define ET_NONE ENTITY_TYPE::none
  ca = 2,
#define ET_CA ENTITY_TYPE::ca
  client = 4,
#define ET_CL ENTITY_TYPE::client
  server = 8,
#define ET_SV ENTITY_TYPE::server
};
static inline std::unordered_map<str, ENTITY_TYPE> entity_type_map{
    {"ca", ET_CA}, {"server", ET_SV}, {"client", ET_CL}, {"sv",ET_SV}, {"cl",ET_CL}};
static inline int operator&(ENTITY_TYPE lo, ENTITY_TYPE ro) { return (ui16)lo & (ui16)ro; }

enum class ENTITY_FIELDS : uint16_t {
  all = 8191,
#define E_ALL ENTITY_FIELDS::all
  none = 0,
#define E_NONE ENTITY_FIELDS::none
  profile = 2,
#define E_PROFILE ENTITY_FIELDS::profile
  subject_cn = 4,
#define E_COMMON ENTITY_FIELDS::subject_cn
  type = 8,
#define E_TYPE ENTITY_FIELDS::type
  serial = 16,
#define E_SERIAL ENTITY_FIELDS::serial
  subject_country = 32,
#define E_COUNTRY ENTITY_FIELDS::subject_country
  subject_state = 64,
#define E_STATE ENTITY_FIELDS::subject_state
  subject_location = 128,
#define E_LOCATION ENTITY_FIELDS::subject_location
  subject_organisation = 256,
#define E_ORG ENTITY_FIELDS::subject_organisation
  subject_email = 512,
#define E_MAIL ENTITY_FIELDS::subject_email
  key_path = 1024,
#define E_KEYPATH ENTITY_FIELDS::key_path
  req_path = 2048,
#define E_REQPATH ENTITY_FIELDS::req_path
  cert_path = 4096,
#define E_CRTPATH ENTITY_FIELDS::cert_path
};
/* ENTITY OPERATORS */
static ENTITY_FIELDS operator|(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<ENTITY_FIELDS>(static_cast<ui16>(lo) |
                                    static_cast<ui16>(ro));
}
static bool operator&(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<ui16>(lo) & static_cast<ui16>(ro);
}
/* ENTITY STRING -> ENTITY_FIELD MAP*/
static inline std::unordered_map<std::string, ENTITY_FIELDS>
entity_fields_map() {
  return {{"profile", ENTITY_FIELDS::profile},
          {"cn", ENTITY_FIELDS::subject_cn},
          {"type", ENTITY_FIELDS::type},
          {"serial", ENTITY_FIELDS::serial},
          {"country", ENTITY_FIELDS::subject_country},
          {"state", ENTITY_FIELDS::subject_state},
          {"location", ENTITY_FIELDS::subject_location},
          {"org", ENTITY_FIELDS::subject_organisation},
          {"mail", ENTITY_FIELDS::subject_email},
          {"key", ENTITY_FIELDS::key_path},
          {"req", ENTITY_FIELDS::req_path},
          {"crt", ENTITY_FIELDS::cert_path}};
}

struct Profile {
  str name;
  str source;
  ui8 ca_created = 0;
  ui16 sv_count = 0;
  ui16 cl_count = 0;

  inline str gopenssl() {
    return fmt::format("{}{}{}", source, SLASH, "gopenssl.conf");
  }
  inline str dir_crl() {
    return fmt::format("{}{}pki{}crl{}", source, SLASH, SLASH, SLASH);
  }
  inline str dir_crt(){
    return fmt::format("{}{}pki{}certs{}",source, SLASH, SLASH, SLASH);
  }
  inline str dir_key(){
    return fmt::format("{}{}pki{}keys{}", source, SLASH, SLASH, SLASH);
  }
  inline str dir_req(){
    return fmt::format("{}{}pki{}reqs{}", source, SLASH, SLASH, SLASH);
  }
  inline str csv_entry() {
    return fmt::format("{},{},{},{},{}", name, source, ca_created, sv_count,
                       cl_count);
  }
  inline str ca_crt() {
    return fmt::format("{}{}pki{}ca{}crt", source, SLASH, SLASH, SLASH);
  }
  inline str ca_key() {
    return fmt::format("{}{}pki{}ca{}key", source, SLASH, SLASH, SLASH);
  }

};


static inline str str_conversion(PROFILE_FIELDS field) {
  return (field & P_NAME ? "name" : "source");
}

static inline str str_conversion(ENTITY_TYPE type) {
  return (type & ET_CA
              ? "ca"
              : (type & ET_SV ? "server" : (type & ET_CL ? "client" : "none")));
};

template <typename T> str to_str(T enumclass) {
  return str_conversion(enumclass);
};

// #define SUBJECT_TEMPLATE "/C=%s/ST=%s/L=%s/O=%s/CN=%s/emailAddress=%s"
struct Subject {
  str country;
  str state;
  str location;
  str organisation;
  str cn;
  str email;
  inline str oneliner() {
    str _subj;
    if(!country.empty()){_subj+="/C="+country;};
    if(!state.empty()){_subj+="/ST="+state;};
    if(!location.empty()){_subj+="/L="+location;};
    if(!organisation.empty()){_subj+="/O="+organisation;};
    if(!cn.empty()){_subj+="/CN="+cn;};
    if(!email.empty()){_subj+="/emailAddress="+email;};
    return std::move(_subj);
  }
  inline void print(){
    fmt::print(fg(LGREEN), "common name: {}\ncountry: {}\nstate: {}\nlocation: {}\norganisation: {}\nemail: {}\n",cn,country,state,location,organisation,email);
  }
};
struct Entity {
  Subject subject;
  ENTITY_TYPE type; // ca-sv-cl
  str serial;
  str key_path;
  str csr_path;
  str crt_path;
  inline str csv_entry() {
    return subject.cn + "," + to_str(type) + "," + serial +
           "," + subject.country + "," + subject.state + "," +
           subject.location + "," + subject.organisation + "," + subject.email + "," + key_path + "," + csr_path + "," + crt_path + ",";
  };
};

}; // namespace gpkih