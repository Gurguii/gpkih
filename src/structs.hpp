#pragma once
#include "gpki.hpp" // typename aliases + fmtlib
#include <unordered_map>
#include <ctime>
#include "utils/utils.hpp"
#include <chrono>

namespace gpkih {
enum class PROFILE_FIELDS : uint16_t {
  all = 511,
#define P_ALL PROFILE_FIELDS::all
  none = 0,
#define P_NONE PROFILE_FIELDS::none
  name = 2,
#define P_NAME PROFILE_FIELDS::name
  source = 4,
#define P_SRC PROFILE_FIELDS::source
  creation_date = 8,
#define P_CREATION_DATE PROFILE_FIELDS::creation_date
  last_modification = 16,
#define P_LAST_MODIFICATION PROFILE_FIELDS::last_modification
  id = 32,
#define P_ID PROFILE_FIELDS::id
  ca_created = 64,
#define P_CA_CREATED PROFILE_FIELDS::ca_created
  sv_count = 128,
#define P_SV_COUNT PROFILE_FIELDS::sv_count
  cl_count = 256,
#define P_CL_COUNT PROFILE_FIELDS::cl_count 
};

/* PROFILE OPERATORS */
static PROFILE_FIELDS operator|(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<PROFILE_FIELDS>((uint16_t)lo | (uint16_t)ro);
};

static bool operator&(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<bool>((uint16_t)lo & (uint16_t)ro);
}

/* PROFILE STRING -> PROFILE_FIELD MAP */
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

static inline std::unordered_map<std::string, ENTITY_TYPE> entity_type_map{
    {"ca", ET_CA}, {"server", ET_SV}, {"client", ET_CL}, {"sv",ET_SV}, {"cl",ET_CL}};
static inline int operator&(ENTITY_TYPE lo, ENTITY_TYPE ro) { return (uint16_t)lo & (uint16_t)ro; }

enum class ENTITY_FIELDS : uint16_t {
  none = 0,
#define E_NONE ENTITY_FIELDS::none
  creation_date = 2,
#define E_CREATION_DATE ENTITY_FIELDS::creation_date
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
  status = 8192,
#define E_STATUS ENTITY_FIELDS::status
  all = 16383,
#define E_ALL ENTITY_FIELDS::all
};

/* ENTITY OPERATORS */
static ENTITY_FIELDS operator|(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<ENTITY_FIELDS>(static_cast<uint16_t>(lo) |
                                    static_cast<uint16_t>(ro));
}
static bool operator&(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro);
}

/* ENTITY STRING -> ENTITY_FIELD MAP*/
static inline std::unordered_map<std::string, ENTITY_FIELDS>
entity_fields_map() {
  return {{"date", E_CREATION_DATE},
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

enum class ENTITY_STATUS : uint8_t {
  none = 0,
  active = 2,
#define ES_ACTIVE ENTITY_STATUS::active
  revoked = 4,
#define ES_REVOKED ENTITY_STATUS::revoked
  marked = 8,
#define ES_MARKED ENTITY_STATUS::marked
  all = 15,
#define ES_ALL ENTITY_STATUS::all
};

static inline std::unordered_map<ENTITY_STATUS, std::string> entity_status_to_str{
  {ENTITY_STATUS::active,"active"},
  {ENTITY_STATUS::revoked,"revoked"},
  {ENTITY_STATUS::marked,"marked"},
};


static inline std::string str_conversion(PROFILE_FIELDS field) {
  return (field & P_NAME ? "name" : "source");
}
static inline std::string str_conversion(ENTITY_TYPE type) {
  return (type & ET_CA
              ? "ca"
              : (type & ET_SV ? "server" : (type & ET_CL ? "client" : "none")));
};
static inline std::string str_conversion(ENTITY_STATUS status){
  auto iter = entity_status_to_str.find(status);
  return iter != entity_status_to_str.end() ? iter->second : "n/a"; 
}

template <typename T> std::string to_str(T enumclass) {
  return str_conversion(enumclass);
};

struct Profile {
  uint64_t id;
  
  char *name = NULL;
  uint8_t namelen;

  char *source = NULL;
  uint8_t sourcelen;

  std::chrono::time_point<std::chrono::system_clock> creation_date = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> last_modification = std::chrono::system_clock::now();

  uint8_t ca_created = 0;
  uint16_t sv_count = 0;
  uint16_t cl_count = 0;
};

// #define SUBJECT_TEMPLATE "/C=%s/ST=%s/L=%s/O=%s/CN=%s/emailAddress=%s"
struct Subject {
  char country[3]{0};

  char *state = NULL;
  uint8_t statelen = 0;

  char *location = NULL;
  uint8_t locationlen = 0; 

  char *organisation = NULL;
  uint8_t organisationlen = 0;
  
  char *cn = NULL;
  uint8_t cnlen = 0;
  
  char *email = NULL;
  uint8_t emaillen = 0;
};

struct Entity {
  Subject subject{};
  
  ENTITY_TYPE type = ENTITY_TYPE::none; // ca-sv-cl
  ENTITY_STATUS status = ENTITY_STATUS::active;
  
  size_t serial = 0;

  std::chrono::time_point<std::chrono::system_clock> creation_date = std::chrono::system_clock::now();

  char *key_path = NULL;
  uint8_t key_path_len = 0;

  char *csr_path = NULL;
  uint8_t csr_path_len = 0;

  char *crt_path = NULL;
  uint8_t crt_path_len = 0;
};

}; // namespace gpkih


namespace gpkih::profile
{
  static inline std::string ca_key(Profile& ref) {
    return fmt::format("{}{}pki{}ca{}key",ref.source, SLASH, SLASH, SLASH);
  }
  static inline fs::path ca_key_fs(Profile& ref) {
    fs::path p{ref.source};
    p/"pki"/"ca"/"key";
    return p;
  }

  static inline std::string ca_crt(Profile& ref) {
      return fmt::format("{}{}pki{}ca{}crt",ref.source, SLASH, SLASH, SLASH);
  }
  
  static inline fs::path ca_crt_fs(Profile& ref) {
    fs::path p(ref.source);
    p/"pki"/"ca"/"crt";
    return p;
  }

  static inline std::string gopenssl(Profile &ref){
    return fmt::format("{}{}{}", ref.source, SLASH, "gopenssl.conf");
  }

  static inline std::string dir_crl(Profile &ref){
    return fmt::format("{}{}pki{}crl{}", ref.source, SLASH, SLASH, SLASH);
  }

  static inline std::string dir_key(Profile &ref){
    return fmt::format("{}{}pki{}keys{}", ref.source, SLASH, SLASH, SLASH);
  }

  static inline fs::path dir_key_fs(Profile& ref) {
    fs::path p(ref.source);
    p/"pki"/"keys";
    return p;
  }

  static inline std::string dir_req(Profile &ref){
    return fmt::format("{}{}pki{}reqs{}", ref.source, SLASH, SLASH, SLASH);
  }
  static inline fs::path dir_req_fs(Profile& ref) {
    fs::path p(ref.source);
    p/"pki"/"reqs";
    return p;
  }

  static inline std::string dir_crt(Profile& ref) {
    return fmt::format("{}{}pki{}certs{}", ref.source, SLASH, SLASH, SLASH);
  }
  static inline fs::path dir_crt_fs(Profile &ref){
    fs::path p(ref.source);
    p/"pki"/"certs";
    return p;
  }
} // namespace gpkih::profile

namespace gpkih::subject
{
  static inline std::string openssl_oneliner(Subject &ref){
    std::string _subj;
    
    if (len(ref.country) != 0){_subj += std::move(fmt::format("/C={}",ref.country)); };
    if (ref.state != NULL){_subj += std::move(fmt::format("/ST={}",ref.state)); };
    if (ref.location != NULL){_subj += std::move(fmt::format("/L={}", ref.location));  };
    if (ref.organisation != NULL){_subj += std::move(fmt::format("/O={}",ref.organisation));};
    if (ref.cn != NULL){_subj += std::move(fmt::format("/CN={}",ref.cn));};
    if (ref.email != NULL){_subj += std::move(fmt::format("/emailAddress={}",ref.email));};

    PDEBUG(3,"openssl oneliner ['{}']", _subj);
    return std::move(_subj);
  }
} // namespace gpkih::subject