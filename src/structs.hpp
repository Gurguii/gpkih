#pragma once
#include <string>
#include "gpki.hpp"

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
  std::string gopenssl(){
    return source + SLASH + "gopenssl.cnf";
  }
  std::string dir_crl(){
    return source + SLASH + "pki" + SLASH + "crl";
  }
  std::string csv_entry(){
    return name + "," + source;
  }
  void empty(){
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
    return "/C=" + country + "/ST=" + state +
           "/L=" + location + "/O=" + organisation +
           "/CN=" + cn + "/emailAddress=" + email;
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
    return profile_name + "," + 
    subject.cn + "," + 
    type + "," +
    subject.country + "," + 
    subject.state + "," + 
    subject.location + "," + 
    subject.organisation + "," +
    subject.email + "," +
    key_path + "," + 
    req_path + "," +
    cert_path;  
  }
};
} // namespace gpki
namespace gpki::subopts
{
  struct init{
    std::string profile_name;
    std::string profile_source;
  }; 
  struct build{
    std::string key_size = "1024";
    std::string algorithm = "rsa";
    std::string key_format = "pem";
    std::string csr_crt_format = "pem";
  };
  
  struct revoke{
    std::string common_name; 
    std::string reason = "not specified";
  };
}
