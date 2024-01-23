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

enum class ENTITY_FIELDS : uint8_t {
  profile = 0,
  subject_cn = 1,
  type = 2,
  subject_country = 3,
  subject_state = 4,
  subject_location = 5,
  subject_organisation = 6,
  subject_email = 7,
  key_path = 8,
  req_path = 9,
  cert_path = 10,
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
enum class PROFILE_FIELDS : uint8_t { name = 0, source = 1 };
inline std::unordered_map<std::string, PROFILE_FIELDS> profile_fields_map() {
  return {
      {"name", PROFILE_FIELDS::name},
      {"source", PROFILE_FIELDS::source},
  };
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
  std::vector<std::string> profile;
  std::vector<std::string> common_name;
  std::vector<ENTITY_FIELDS> efields;
  std::vector<PROFILE_FIELDS> pfields;
};
} // namespace gpki::subopts
/*
 *std::string country = "ES";
  std::string state = "GRAN CANARIAS";
  std::string location = "LAS PALMAS";
  std::string organisation = "MARIWANOS";
  std::string cn;
  std::string email = "NONE";
  std::string oneliner() {
*/
