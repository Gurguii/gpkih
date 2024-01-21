#pragma once
#include <cstdint>
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
/*
 *std::string country = "ES";
  std::string state = "GRAN CANARIAS";
  std::string location = "LAS PALMAS";
  std::string organisation = "MARIWANOS";
  std::string cn;
  std::string email = "NONE";
  std::string oneliner() {
*/
enum class ENTITY_FIELDS : uint16_t{
  all = 0,
  subject = 2,
  subject_country = 4,
  subject_state = 8,
  subject_location = 16,
  subject_organisation = 32,
  subject_cn = 64,
  subject_email = 128,
  key_path = 256,
  cert_path = 512,
  req_path = 1024,
};
enum class PROFILE_FIELDS : uint8_t {
  all = 0,
  name = 2,
  source = 4
};
  struct gencrl{
    /* No subopts */
  };
  // ./gpki info -p test | give info about profile test
  // ./gpki info -p test -cn client1 | give info about 'client1' from profile 'test'
  // ./gpki info -p | give info about all existing profiles
  // ./gpki info -p -cn | give info about all profiles and all entities
  struct list{
    std::string profile;
    std::string common_name;
    ENTITY_FIELDS einfo; 
    PROFILE_FIELDS pinfo;
  };
}
