#pragma once

#include "../config/config_management.hpp"
#include "../gpkih.hpp"
#include "../printing/printing.hpp"
#include "../logger/logger.hpp"
#include "../db/profiles.hpp"
#include "../db/entities.hpp"
#include "../utils/utils.hpp"
#include "../experimental/formatter.hpp"

namespace gpkih::actions {
extern int init(std::string_view &profile_name, std::string_view &profile_source);

extern int revoke(Profile &profile, std::vector<std::string> &common_names, std::vector<std::string> &serials);

extern int gencrl(Profile &profile);

extern int list_profiles(uint16_t fields);
extern int list_entities(std::string_view profile_name, uint16_t fields);

extern int remove(std::vector<std::string> &profiles_to_remove, int remove_all = 0);

extern int build_ca(Profile &profile, ProfileConfig &pkiconf, Entity &entity, EntityManager &eman, std::string_view days, std::string_view keyAlgo, std::string_view keySize);
extern int build(Profile &profile, ProfileConfig &config, Entity &entity, EntityManager &eman, std::string_view days, std::string_view keyAlgo, std::string_view keySize);
} // namespace gpkih::actions