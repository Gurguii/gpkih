#pragma once
#include <cstdint>

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
static inline PROFILE_FIELDS operator|(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<PROFILE_FIELDS>((uint16_t)lo | (uint16_t)ro);
};

static inline bool operator&(PROFILE_FIELDS lo, PROFILE_FIELDS ro) {
  return static_cast<bool>((uint16_t)lo & (uint16_t)ro);
}

static inline bool operator&(uint16_t lo, PROFILE_FIELDS ro) {
  return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
}