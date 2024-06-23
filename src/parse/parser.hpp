#pragma once
#include <unordered_map>
#include <vector>
#include <string>
#include "../actions/actions.hpp"
#include "../db/profiles.hpp"

namespace gpkih::parsers 
{

// TESTING
extern int parseGlobals(std::vector<std::string> &opts);

/// @brief does generic parsing and calls specialized parser
extern int parse(std::vector<std::string> &opts);

/// @brief initialize a new profile, creating the PKI file structure
extern int init(std::vector<std::string> &opts);

/// @brief build ca|server|client key + certificates
extern int build(std::vector<std::string> &opts);

/// @brief revoke certificate
extern int revoke(std::vector<std::string> &opts);

/// @brief generate certificate revocation list
extern int gencrl(std::vector<std::string> &opts);

/// @brief list profiles/entities
extern int list(std::vector<std::string> &opts);

/// @brief remove remove profile
extern int remove(std::vector<std::string> &opts);

/// @brief rename profile
extern int rename(std::vector<std::string> &opts);

/// @brief remove gpkih base dir and all profiles
extern int reset(std::vector<std::string> &opts);

/// @brief retrieve general/profile configuration
extern int get(std::vector<std::string> &opts);

/// @brief change general/profile configuration
extern int set(std::vector<std::string> &opts);

/// @brief generate key
// unimplementes
extern int genkey(std::vector<std::string> &opts);
} // namespace gpkih::parsers

inline std::unordered_map<std::string, int (*)(std::vector<std::string> &opts)> ACTION_PARSERS
{
  {"init", gpkih::parsers::init},     {"list", gpkih::parsers::list},
  {"build", gpkih::parsers::build},   {"revoke", gpkih::parsers::revoke},
  {"gencrl", gpkih::parsers::gencrl}, {"remove", gpkih::parsers::remove},
  {"genkey", gpkih::parsers::genkey}, {"get", gpkih::parsers::get}, {"set", gpkih::parsers::set},
  {"rename", gpkih::parsers::rename}, {"reset", gpkih::parsers::reset},
};