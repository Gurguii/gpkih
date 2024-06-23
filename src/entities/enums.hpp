#pragma once
#include <cstdint>

enum class ENTITY_TYPE : uint8_t {
  none   = 0,
  ca     = 1,
  client = 2,
  server = 4,
  all    = 7
};
constexpr ENTITY_TYPE ET_NONE = ENTITY_TYPE::none;
constexpr ENTITY_TYPE ET_CA   = ENTITY_TYPE::ca;
constexpr ENTITY_TYPE ET_CL   = ENTITY_TYPE::client;
constexpr ENTITY_TYPE ET_SV   = ENTITY_TYPE::server;

static inline bool operator&(ENTITY_TYPE lo, ENTITY_TYPE ro) { return static_cast<bool>((uint8_t)lo & (uint8_t)ro); }

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
constexpr ENTITY_FIELDS E_NONE          =  ENTITY_FIELDS::none;
constexpr ENTITY_FIELDS E_CREATION_DATE =  ENTITY_FIELDS::creationDate;
constexpr ENTITY_FIELDS E_COMMON        =  ENTITY_FIELDS::subject_cn;
constexpr ENTITY_FIELDS E_TYPE          =  ENTITY_FIELDS::type;
constexpr ENTITY_FIELDS E_SERIAL        =  ENTITY_FIELDS::serial;
constexpr ENTITY_FIELDS E_COUNTRY       =  ENTITY_FIELDS::subject_country;
constexpr ENTITY_FIELDS E_STATE         =  ENTITY_FIELDS::subject_state;
constexpr ENTITY_FIELDS E_LOCATION      =  ENTITY_FIELDS::subject_location;
constexpr ENTITY_FIELDS E_ORG           =  ENTITY_FIELDS::subject_organisation;
constexpr ENTITY_FIELDS E_MAIL          =  ENTITY_FIELDS::subject_email;
constexpr ENTITY_FIELDS E_STATUS        =  ENTITY_FIELDS::status;
constexpr ENTITY_FIELDS E_KEYPATH       =  ENTITY_FIELDS::keyPath;
constexpr ENTITY_FIELDS E_REQPATH       =  ENTITY_FIELDS::req_path;
constexpr ENTITY_FIELDS E_CRTPATH       =  ENTITY_FIELDS::cert_path;
constexpr ENTITY_FIELDS E_ALL           =  ENTITY_FIELDS::all;
constexpr ENTITY_FIELDS E_EXPIRATION    =  ENTITY_FIELDS::expirationDate;

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

enum class ENTITY_STATUS : uint8_t {
  none    = 0,
  active  = 1,
  revoked = 2,
  marked  = 4,
  all     = 7,
};
constexpr ENTITY_STATUS ES_NONE    = ENTITY_STATUS::none;
constexpr ENTITY_STATUS ES_ACTIVE  = ENTITY_STATUS::active;
constexpr ENTITY_STATUS ES_REVOKED = ENTITY_STATUS::revoked;
constexpr ENTITY_STATUS ES_MARKED  = ENTITY_STATUS::marked;
constexpr ENTITY_STATUS ES_ALL     = ENTITY_STATUS::all;