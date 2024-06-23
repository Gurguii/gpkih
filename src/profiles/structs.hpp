#pragma once
#include <cstdint>
#include <fmt/chrono.h>

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
  uint64_t id = 0;
  
  char *name = nullptr;
  uint8_t namelen = 0;

  char *source = nullptr;
  uint8_t sourcelen = 0;

  std::chrono::time_point<std::chrono::system_clock> creationDate = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> last_modification = std::chrono::system_clock::now();

  uint8_t ca_created = 0;
  uint16_t sv_count  = 0;
  uint16_t cl_count  = 0;

  Profile() = default;
};