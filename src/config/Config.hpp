#pragma once
#include "enums.hpp"
#include <unordered_map>
#include "../entities/entities.hpp"

namespace gpkih::Config 
{
  // Namespace to manage gpkih.conf
  extern ConfigMap gpkihConfig;

  /// @brief load configuration from given path
  /// @param filepath configuration file path
  /// @return GPKIH_OK if file was succesfully loaded else GPKIH_FAIL
  /// @note this file will only load 1 filepath, no matter how many times it gets called
  extern int load(std::string_view filepath);

  /// @brief get the value of a section's key
  /// @return std::string_view with the value
  /// @note the returned value could be empty if the key does not exist
  extern std::string_view get(std::string_view section, std::string_view key);

  /// @brief set a new value of a section key
  /// @return GPKIH_OK if value was changed, GPKIH_FAIL if it didn't
  extern int set(std::string_view section, std::string_view key, std::string_view val);

  /// @brief checks if section exists
  /// @return true if section exists else false
  extern bool section_exists(const char * section);

  /// @brief checks if key exists in any section
  /// @return true if key exists else false
  extern const std::unordered_map<std::string,std::string>* const key_exists(const char * key);

  /// @brief checks if key exists in a specific section
  /// @return true if key exists else false
  extern bool key_exists(const char * section, const char * key);

  /// @brief writes configuration to file, updating values made with Config::set() since it was loaded
  /// @return GPKIH_OK if succeeds else GPKIH_FAIL
  extern int sync();
}; // namespace gpkih::Config