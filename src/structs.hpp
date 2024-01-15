#pragma once
#include <string>

namespace gpki {

struct ProfileStatus {
  int ca_count = 0x00;
  int sv_count = 0x00;
  int cl_coint = 0x00;
};

struct Profile {
  std::string name;
  std::string source;
  ProfileStatus status;
};

// #define SUBJECT_TEMPLATE "/C=%s/ST=%s/L=%s/O=%s/CN=%s/emailAddress=%s"
struct Subject {
  std::string country = "ES";
  std::string state = "GRAN CANARIAS";
  std::string location = "LAS PALMAS";
  std::string organisation = "MARIWANOS";
  std::string cn;
  std::string email = "none@example.com";
  std::string oneliner() {
    return "'/C=" + country + "/ST=" + state +
           "/L=" + location + "/O=" + organisation +
           "/CN=" + cn + "/emailAddress=" + email + "'";
  }
};

struct Entity {
  Subject subject;
  std::string key_path;
  std::string req_path;
  std::string cert_path;
  std::string profile_name;
  std::string type; // ca-sv-cl
  std::string csv_entry(){
    return "profile_name," + profile_name 
    + "common_name," + subject.cn 
    + "type," + type 
    + "country," + subject.country
    + "state," + subject.state
    + "location," + subject.location
    + "organisation," + subject.organisation
    + "email," + subject.email
    + "key_path," + key_path
    + "req_path," + req_path
    + "cert_path," + cert_path; 
  }
};
} // namespace gpki
