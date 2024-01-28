#pragma once
#include "gpki.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
namespace gpki {

struct ProfileStatus {
  int ca_count = 0x00;
  int sv_count = 0x00;
  int cl_count = 0x00;
};

struct Profile {
  std::string name;
  std::string source;
  ProfileStatus status;
  std::string gopenssl() { return source + SLASH + "gopenssl.cnf"; }
  std::string dir_crl() { return source + SLASH + "pki" + SLASH + "crl"; }
  std::string csv_entry() { return name + "," + source; }
  void empty() {
    name.assign("");
    source.assign("");
    status.ca_count = 0;
    status.sv_count = 0;
    status.cl_count = 0;
  }
};

// #define SUBJECT_TEMPLATE "/C=%s/ST=%s/L=%s/O=%s/CN=%s/emailAddress=%s"
struct Subject {
  std::string country = "ES";
  std::string state = "GRAN CANARIAS";
  std::string location = "LAS PALMAS";
  std::string organisation = "MARIWANOS";
  std::string cn;
  std::string email = "NONE";
  std::string oneliner() {
    return "/C=" + country + "/ST=" + state + "/L=" + location +
           "/O=" + organisation + "/CN=" + cn + "/emailAddress=" + email;
  }
};

struct Entity {
  Subject subject;
  std::string key_path;
  std::string req_path;
  std::string cert_path;
  std::string profile_name;
  std::string type; // ca-sv-cl
  std::string csv_entry() {
    return profile_name + "," + subject.cn + "," + type + "," +
           subject.country + "," + subject.state + "," + subject.location +
           "," + subject.organisation + "," + subject.email + "," + key_path +
           "," + req_path + "," + cert_path;
  }
};
} // namespace gpki

enum class ENTITY_TYPE { none, ca, sv, cl };
const auto entity_type_str = [](ENTITY_TYPE type) {
  return (type == ENTITY_TYPE::ca
              ? "ca"
              : (type == ENTITY_TYPE::sv ? "server" : "client"));
};

enum class ENTITY_FIELDS : uint16_t {
  all = 4096,
#define E_ALL ENTITY_FIELDS::all
  none = 0,
#define E_NONE ENTITY_FIELDS::none
  profile = 2,
#define E_PROFILE ENTITY_FIELDS::profile
  subject_cn = 4,
#define E_COMMON ENTITY_FIELDS::subject_cn
  type = 8,
#define E_TYPE ENTITY_FIELDS::type
  subject_country = 16,
#define E_COUNTRY ENTITY_FIELDS::subject_country
  subject_state = 32,
#define E_STATE ENTITY_FIELDS::subject_state
  subject_location = 64,
#define E_LOCATION ENTITY_FIELDS::subject_location
  subject_organisation = 128,
#define E_ORG ENTITY_FIELDS::subject_organisation
  subject_email = 256,
#define E_MAIL ENTITY_FIELDS::subject_email
  key_path = 512,
#define E_KEYPATH ENTITY_FIELDS::key_path
  req_path = 1024,
#define E_REQPATH ENTITY_FIELDS::req_path
  cert_path = 2048,
#define E_CRTPATH ENTITY_FIELDS::cert_path
};

inline std::unordered_map<std::string, ENTITY_FIELDS> entity_fields_map() {
  return {{"profile", ENTITY_FIELDS::profile},
          {"cn", ENTITY_FIELDS::subject_cn},
          {"type", ENTITY_FIELDS::type},
          {"country", ENTITY_FIELDS::subject_country},
          {"state", ENTITY_FIELDS::subject_state},
          {"location", ENTITY_FIELDS::subject_location},
          {"org", ENTITY_FIELDS::subject_organisation},
          {"mail", ENTITY_FIELDS::subject_email},
          {"key", ENTITY_FIELDS::key_path},
          {"req", ENTITY_FIELDS::req_path},
          {"crt", ENTITY_FIELDS::cert_path}};
}

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

inline std::unordered_map<std::string, PROFILE_FIELDS> profile_fields_map() {
  return {
      {"name", PROFILE_FIELDS::name},
      {"source", PROFILE_FIELDS::source},
  };
}

/* PROFILE OPERATORS */
static PROFILE_FIELDS operator|(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<PROFILE_FIELDS>((ui16)lo | (ui16)ro);
};
static bool operator&(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return (ui16)lo & (ui16)ro;
}
/* ENTITY OPERATORS */
static ENTITY_FIELDS operator|(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<ENTITY_FIELDS>((ui16)lo | (ui16)ro);
}
static bool operator&(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return (ui16)lo | (ui16)ro;
}

namespace gpki::subopts {
struct init {
  std::string profile_name;
  std::string profile_source;
};

struct build {
  std::string key_size = "1024";
  std::string algorithm = "rsa";
  std::string key_format = "pem";
  std::string csr_crt_format = "pem";
};

struct revoke {
  std::string common_name;
  std::string reason = "not specified";
};

struct gencrl {
  /* No subopts */
};

struct list {
  std::vector<std::string> profiles;
  std::vector<std::string> entities;

  PROFILE_FIELDS pfields = P_ALL;
  ENTITY_FIELDS efields = E_ALL;
};
} // namespace gpki::subopts