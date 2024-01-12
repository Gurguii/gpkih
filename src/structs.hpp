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

struct Subject {
  std::string country;
  std::string state;
  std::string location;
  std::string organisation;
  std::string cn;
  std::string email;
};
// #define SUBJECT_TEMPLATE "/C=%s/ST=%s/L=%s/O=%s/CN=%s/emailAddress=%s"
struct Entity {
  Subject subject;
  std::string key_path;
  std::string req_path;
  std::string cert_path;
  std::string profile_name;
  std::string subj_oneliner() {
    return "'/C=" + subject.country + "/ST=" + subject.state +
           "/L=" + subject.location + "/O=" + subject.organisation +
           "/CN=" + subject.cn + "/emailAddress=" + subject.email + "'";
  };
};
} // namespace gpki
