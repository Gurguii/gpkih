#pragma once
#include <cstdint>
#include <fmt/chrono.h>

/**
 * @struct ProfileMetadata
 * @brief Holds Profile Metadata.
 * @var Profile::id 
 * Unique identifier for the profile.  
 * @var Profile::namelen 
 * Length of the name (excluding null terminator).
 * @var Profile::sourcelen 
 * Length of the source (excluding null terminator).
 * @var Profile::creationDate 
 * Time when the profile was created.
 * @var Profile::lastModification 
 * Time of the last profile modification.
 * @var Profile::caCreated 
 * Flag indicating if a Certificate Authority was created for this profile.
 * @var Profile::svCount 
 * Number of server certificates associated with the profile.
 * @var Profile::clCount 
 * Number of client certificates associated with the profile.
*/
struct ProfileMetadata{
  uint64_t id = 0;
  uint8_t nameLen = 0;
  uint16_t sourceLen = 0;
  std::chrono::time_point<std::chrono::system_clock> creationDate = std::chrono::system_clock::now();
  std::chrono::time_point<std::chrono::system_clock> lastModification = std::chrono::system_clock::now();
  uint8_t caCreated = 0;
  uint16_t svCount  = 0;
  uint16_t clCount  = 0;
  ProfileMetadata() = default;
};
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
 * @var Profile::lastModification 
 * Time of the last profile modification.
 * @var Profile::caCreated 
 * Flag indicating if a Certificate Authority was created for this profile.
 * @var Profile::sv_count 
 * Number of server certificates associated with the profile.
 * @var Profile::cl_count 
 * Number of client certificates associated with the profile.
*/
struct Profile {
  const char *name = nullptr;
  const char *source = nullptr;
  ProfileMetadata meta;
  Profile() = default;
};