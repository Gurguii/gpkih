#pragma once
#include <string>

namespace gpki {

struct Profile {
  int id;
  std::string name;
  std::string source;
};
struct Subject {
  std::string country;
  std::string state;
  std::string location;
  std::string organisation;
  std::string cn;
  std::string email;
  std::string serial;
};
// #define SUBJECT_TEMPLATE "/C=%s/ST=%s/L=%s/O=%s/CN=%s/emailAddress=%s"
struct Entity {
  Subject subject;
  std::string serial;
  std::string cert_path;
  std::string key_path;
  std::string csr_path;
  int profile_id;
  std::string subj_oneliner() {
    return "'/C=" + subject.country + "/ST=" + subject.state +
           "/L=" + subject.location + "/O=" + subject.organisation +
           "/CN=" + subject.cn + "/emailAddress=" + subject.email + "'";
  };
};
} // namespace gpki
