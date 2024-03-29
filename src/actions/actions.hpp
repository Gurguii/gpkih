#pragma once
#include "../config/config_management.hpp"
#include "../db/database.hpp"
#include "../gpki.hpp"
#include "../printing/printing.hpp"
#include "../logger/logger.hpp"

namespace gpkih::actions {
int init(strview profile_name, strview profile_source);

int revoke(Profile &profile, std::vector<str> &common_names, std::vector<str> &serials);

int gencrl(Profile &profile);

int list(std::vector<str> &profiles, std::vector<str> &entities, PROFILE_FIELDS pfields = P_ALL, ENTITY_FIELDS efields = E_ALL);

int remove(std::vector<str> &profiles_to_remove, int remove_all = 0);

int build_ca(Profile &profile, ProfileConfig &pkiconf, Entity &entity);
int build(Profile &profile, ProfileConfig &config, Entity &entity);
template <typename ...T> int build(T&& ...args); // build forwarder
} // namespace gpkih::actions