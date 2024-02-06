#pragma once
#include "gpki.hpp"
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace gpki {
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
static inline std::unordered_map<std::string, PROFILE_FIELDS> profile_fields_map() {
  return {
      {"name", PROFILE_FIELDS::name},
      {"source", PROFILE_FIELDS::source},
  };
}
enum class ENTITY_TYPE { 
  none = 0, 
  #define ET_NONE  ENTITY_TYPE::none
  ca = 2, 
  #define ET_CA ENTITY_TYPE::ca
  client = 4,
  #define ET_CL ENTITY_TYPE::client
  server = 8,
  #define ET_SV ENTITY_TYPE::server
};
std::unordered_map<str,ENTITY_TYPE>entity_type_map{
  {"ca",ET_CA},
  {"server",ET_SV},
  {"client",ET_CL}
};
int operator&(ENTITY_TYPE lo,ENTITY_TYPE ro){
  return (ui16)lo & (ui16)ro;
}

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
  return static_cast<ENTITY_FIELDS>(static_cast<ui16>(lo) | static_cast<ui16>(ro));
}
static bool operator&(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<ui16>(lo) & static_cast<ui16>(ro);
}
/* ENTITY STRING -> ENTITY_FIELD MAP*/
static inline std::unordered_map<std::string, ENTITY_FIELDS> entity_fields_map() {
  return {{"profile", ENTITY_FIELDS::profile},
          {"cn", ENTITY_FIELDS::subject_cn},
          {"type", ENTITY_FIELDS::type},
          {"serial",ENTITY_FIELDS::serial},
          {"country", ENTITY_FIELDS::subject_country},
          {"state", ENTITY_FIELDS::subject_state},
          {"location", ENTITY_FIELDS::subject_location},
          {"org", ENTITY_FIELDS::subject_organisation},
          {"mail", ENTITY_FIELDS::subject_email},
          {"key", ENTITY_FIELDS::key_path},
          {"req", ENTITY_FIELDS::req_path},
          {"crt", ENTITY_FIELDS::cert_path}};
}

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

static inline str str_conversion(PROFILE_FIELDS field){
  return (field & P_NAME ? "name" : "source");
}
static inline str str_conversion(ENTITY_TYPE type){
  return (type & ET_CA
              ? "ca" : (type & ET_SV 
              ? "server" : (type & ET_CL 
              ? "client" : "none")));
};
template <typename T> str to_str(T enumclass){
  return str_conversion(enumclass);
};

struct Entity {
  Subject subject;
  std::string key_path;
  std::string req_path;
  std::string cert_path;
  std::string profile_name;
  ENTITY_TYPE type; // ca-sv-cl
  std::string serial;
  std::string csv_entry() {
    return profile_name + "," + subject.cn + "," + to_str(type) + "," + serial + "," +
           subject.country + "," + subject.state + "," + subject.location +
           "," + subject.organisation + "," + subject.email + "," + key_path +
           "," + req_path + "," + cert_path;
  }
};

/* UNUSED */
enum class vpn_cl_conf : ui64 
{
  remote = 2,
  remote_cert_tls = 4,
};
enum class vpn_cl_conf_opt : ui64 
{
  installdir = 8,
  verb = 16,
};
enum class vpn_sv_conf : ui64
{
  virtual_server = 32,
  listen_port = 64,
  status = 128,
  explicit_exit_notify = 256,
  keepalive = 512,
  crl_verify = 1024
};
enum class vpn_sv_conf_opt : ui64
{
  dh = 2048,
  ifconfig_pool_persist = 4096,
  push = 8192,
  verb = 16384
};
enum class vpn_common_conf : ui64 {
  protocol = 32768,
  device = 65536,
  cipher = 131072,
  tls = 262144,
};

std::unordered_map<str,ui64> vpn_conf_map{
  {"remote",static_cast<ui64>(vpn_cl_conf::remote)},
  {"remote-cert-tls",static_cast<ui64>(vpn_cl_conf::remote_cert_tls)},
  {"installdir",static_cast<ui64>(vpn_cl_conf_opt::installdir)},
  {"client.verbose",static_cast<ui64>(vpn_cl_conf_opt::verb)},
  {"server",static_cast<ui64>(vpn_sv_conf::virtual_server)},
  {"port",static_cast<ui64>(vpn_sv_conf::listen_port)},
  {"status",static_cast<ui64>(vpn_sv_conf::status)},
  {"explicit-exit-notify",static_cast<ui64>(vpn_sv_conf::explicit_exit_notify)},
  {"keepalive",static_cast<ui64>(vpn_sv_conf::keepalive)},
  {"crl-verify",static_cast<ui64>(vpn_sv_conf::crl_verify)},
{"dh",static_cast<ui64>(vpn_sv_conf_opt::dh)},
{"ifconfig-pool-persist",static_cast<ui64>(vpn_sv_conf_opt::ifconfig_pool_persist)},
{"push",static_cast<ui64>(vpn_sv_conf_opt::push)},
{"server.verbose",static_cast<ui64>(vpn_sv_conf_opt::verb)}
};
}; // namespace gpki