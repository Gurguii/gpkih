#pragma once
#include <cstdint>
#include <ctime>
#include <chrono>
#include <fmt/format.h>

#include "gpkih.hpp"

namespace gpkih {

 /**
 * @struct Profile
 * @brief Represents a profile which internally represents a PKI.
 * @var Profile::id 
 * Unique identifier for the profile.  
 * @var Profile::name 
 * Unique profile name.
 * @var Profile::namelen 
 * Length of the name (excluding null terminator).
 * @var Profile::source 
 * Source of the profile.
 * @var Profile::sourcelen 
 * Length of the source (excluding null terminator).
 * @var Profile::creationDate 
 * Time when the profile was created.
 * @var Profile::last_modification 
 * Time of the last profile modification.
 * @var Profile::ca_created 
 * Flag indicating if a Certificate Authority was created for this profile.
 * @var Profile::sv_count 
 * Number of server certificates associated with the profile.
 * @var Profile::cl_count 
 * Number of client certificates associated with the profile.
*/
struct Profile {
  uint64_t id;
  
  char *name = nullptr;
  uint8_t namelen;

  char *source = nullptr;
  uint8_t sourcelen;

  std::chrono::time_point<std::chrono::system_clock> creationDate = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> last_modification = std::chrono::system_clock::now();

  uint8_t ca_created = 0;
  uint16_t sv_count = 0;
  uint16_t cl_count = 0;
};

enum class PROFILE_FIELDS : uint16_t {
  all = 511,
  none = 0,
  name = 2,
  source = 4,
  creationDate = 8,
  last_modification = 16,
  id = 32,
  ca_created = 64,
  sv_count = 128,
  cl_count = 256,
};

constexpr PROFILE_FIELDS P_ALL = PROFILE_FIELDS::all;
constexpr PROFILE_FIELDS P_NONE = PROFILE_FIELDS::none;
constexpr PROFILE_FIELDS P_NAME = PROFILE_FIELDS::name;
constexpr PROFILE_FIELDS P_SRC = PROFILE_FIELDS::source;
constexpr PROFILE_FIELDS P_CREATION_DATE = PROFILE_FIELDS::creationDate;
constexpr PROFILE_FIELDS P_LAST_MODIFICATION = PROFILE_FIELDS::last_modification;
constexpr PROFILE_FIELDS P_ID = PROFILE_FIELDS::id;
constexpr PROFILE_FIELDS P_CA_CREATED = PROFILE_FIELDS::ca_created;
constexpr PROFILE_FIELDS P_SV_COUNT = PROFILE_FIELDS::sv_count;
constexpr PROFILE_FIELDS P_CL_COUNT = PROFILE_FIELDS::cl_count;

/* PROFILE OPERATORS */
static PROFILE_FIELDS operator|(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<PROFILE_FIELDS>((uint16_t)lo | (uint16_t)ro);
};
static bool operator&(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<bool>((uint16_t)lo & (uint16_t)ro);
}
static bool operator&(uint16_t lo, PROFILE_FIELDS ro) {
  return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
}

static inline std::string to_str(PROFILE_FIELDS field) {
    return (field & P_NAME ? "name" : "source");
}

enum class ENTITY_TYPE {
  none = 0,
  ca = 2,
  client = 4,
  server = 8,
};
constexpr ENTITY_TYPE ET_NONE = ENTITY_TYPE::none;
constexpr ENTITY_TYPE ET_CA = ENTITY_TYPE::ca;
constexpr ENTITY_TYPE ET_CL = ENTITY_TYPE::client;
constexpr ENTITY_TYPE ET_SV = ENTITY_TYPE::server;

static inline int operator&(ENTITY_TYPE lo, ENTITY_TYPE ro) { return (uint16_t)lo & (uint16_t)ro; }

enum class ENTITY_FIELDS : uint16_t {
  none = 0,
  creationDate = 2,
  subject_cn = 4,
  type = 8,
  serial = 16,
  subject_country = 32,
  subject_state = 64,
  subject_location = 128,
  subject_organisation = 256,
  subject_email = 512,
  status = 1024,
  expirationDate = 2048,
  keyPath = 4096,
  req_path = 8192,
  cert_path = 16384,
  all = 32763
};

constexpr ENTITY_FIELDS E_NONE =  ENTITY_FIELDS::none;
constexpr ENTITY_FIELDS E_CREATION_DATE =  ENTITY_FIELDS::creationDate;
constexpr ENTITY_FIELDS E_COMMON =  ENTITY_FIELDS::subject_cn;
constexpr ENTITY_FIELDS E_TYPE =  ENTITY_FIELDS::type;
constexpr ENTITY_FIELDS E_SERIAL =  ENTITY_FIELDS::serial;
constexpr ENTITY_FIELDS E_COUNTRY =  ENTITY_FIELDS::subject_country;
constexpr ENTITY_FIELDS E_STATE =  ENTITY_FIELDS::subject_state;
constexpr ENTITY_FIELDS E_LOCATION =  ENTITY_FIELDS::subject_location;
constexpr ENTITY_FIELDS E_ORG =  ENTITY_FIELDS::subject_organisation;
constexpr ENTITY_FIELDS E_MAIL =  ENTITY_FIELDS::subject_email;
constexpr ENTITY_FIELDS E_STATUS =  ENTITY_FIELDS::status;
constexpr ENTITY_FIELDS E_KEYPATH =  ENTITY_FIELDS::keyPath;
constexpr ENTITY_FIELDS E_REQPATH =  ENTITY_FIELDS::req_path;
constexpr ENTITY_FIELDS E_CRTPATH =  ENTITY_FIELDS::cert_path;
constexpr ENTITY_FIELDS E_ALL =  ENTITY_FIELDS::all;
constexpr ENTITY_FIELDS E_EXPIRATION = ENTITY_FIELDS::expirationDate;

/* ENTITY OPERATORS */
static ENTITY_FIELDS operator|(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<ENTITY_FIELDS>(static_cast<uint16_t>(lo) |
                                    static_cast<uint16_t>(ro));
}
static bool operator&(ENTITY_FIELDS lo, ENTITY_FIELDS ro) {
  return static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro);
}
static bool operator&(uint16_t lo, ENTITY_FIELDS ro){
  return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
}

/* ENTITY STRING -> ENTITY_FIELD MAP*/


enum class ENTITY_STATUS : uint8_t {
  active = 2,
  revoked = 4,
  marked = 8,
  all = 15,
};
constexpr ENTITY_STATUS ES_ACTIVE = ENTITY_STATUS::active;
constexpr ENTITY_STATUS ES_REVOKED = ENTITY_STATUS::revoked;
constexpr ENTITY_STATUS ES_MARKED = ENTITY_STATUS::marked;
constexpr ENTITY_STATUS ES_ALL = ENTITY_STATUS::all;

static inline std::string to_str(ENTITY_TYPE type) {
  switch(type){
  case ET_CA:
    return "ca";
  case ET_CL:
    return "cl";
  case ET_SV:
    return "sv";
  default:
    return "none";
  }
};

static inline std::string to_str(ENTITY_STATUS status){
  switch(status){
    case ES_ACTIVE:
      return "active";
    case ES_REVOKED:
      return "revoked";
    case ES_MARKED:
      return "marked";
    default:
      return "unknown";
  }
}

/**
 * @struct Subject
 * @brief Information about the subject of an Entity
 * @var Subject::country 3 letter code indicating country
 * @var Subject::state state
 * @var Subject::statelen state len
 * @var Subject::location location
 * @var Subject::locationlen location len
 * @var Subject::organisation organisation
 * @var Subject::organisationlen organisation len
 * @var Subject::cn common name (unique per profile)
 * @var Subject::cnlen common name len
 * @var Subject::email email 
 * @var Subject::emaillen email len
 * */
struct Subject {
  char country[3]{0};

  char *state = nullptr;
  uint8_t statelen = 0;

  char *location = nullptr;
  uint8_t locationlen = 0; 

  char *organisation = nullptr;
  uint8_t organisationlen = 0;
  
  char *cn = nullptr;
  uint8_t cnlen = 0;
  
  char *email = nullptr;
  uint8_t emaillen = 0;
};

/**
 * @struct keyInfo
 * @brief holds metadata about Entity's key
 * @var keyInfo::algo key algorithm, ed25519, rsa ...
 * @var keyInfo::algoLen len of keyInfo::algo  
 * @var keyInfo::keySize size of key (bits)
* */ 
struct keyInfo{
  char *algo = nullptr; // ed25519, rsa...
  uint8_t algoLen = 0;
  size_t size;
};

// Idk about this, you can retrieve days by substracting expirationDate and creationDate...
// the x509 extensions are hardcoded for clients | servers | ca's (no need to store it)
// what else?? 
struct crtInfo{
  int days = 0;
};

/**
 * @struct Entity
 * @brief represents the entity behind each created certificate, holds information about the entity itself and the subject
 * @var Entity::subject see \ref Subject
 * @var Entity::type custom enum type indicating the type of entity, valid types are CA,SV,CL, values also include NONE
 * @var Entity::status custom enum type indicating entity status, valid types are ACTIVE,REVOKED,MARKED(not implemented a way to mark yet)
 * @var Entity::serial unique serial
 * @var Entity::creationDate time when Entity was created @note not a string type
 * @var Entity::keyPath absolute path of file containing private key
 * @var Entity::keyPathLen size of keyPath
 * @var Entity::csrPath absolute path of file containing the certificate signing request
 * @var Entity::csrPathLen size of csrPath
 * @var Entity::crtPath absolute path of file containing the signed certificate
 * @var Entity::crtPathLen size of crtPath
 * */
struct Entity {
  Subject subject{};
  
  ENTITY_TYPE type = ET_NONE;
  ENTITY_STATUS status = ES_ACTIVE;
  
  size_t serial = 0;

  std::chrono::time_point<std::chrono::system_clock> creationDate = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> expirationDate; // set by build action based on days configuration

  char *keyPath = nullptr;
  uint32_t keyPathLen = 0;

  char *csrPath = nullptr;
  uint32_t csrPathLen = 0;

  char *crtPath = nullptr;
  uint32_t crtPathLen = 0;

  // !NEW-UNIMPLEMENTED - these members aren't loaded into/from data files
  keyInfo *keyMetadata;
  crtInfo *crtMetadata;

  Entity() = default;
};

}; // namespace gpkih

namespace gpkih::profile
{
  static inline std::string ca_crt(Profile& ref) {
      return fmt::format("{}{}pki{}ca{}crt.pem",ref.source, SLASH, SLASH, SLASH);
  }
  
  static inline std::string gopenssl(Profile &ref){
    return fmt::format("{}{}{}", ref.source, SLASH, "gopenssl.conf");
  }

  static inline fs::path dir_crl_fs(Profile &ref){
    return fs::path{ref.source}/"pki"/"crl";
  }

  static inline std::string dir_key(Profile &ref){
    return fmt::format("{}{}pki{}keys{}", ref.source, SLASH, SLASH, SLASH);
  }

  static inline std::string dir_req(Profile &ref){
    return fmt::format("{}{}pki{}reqs{}", ref.source, SLASH, SLASH, SLASH);
  }

  static inline std::string dir_crt(Profile& ref) {
    return fmt::format("{}{}pki{}certs{}", ref.source, SLASH, SLASH, SLASH);
  }
} // namespace gpkih::profile