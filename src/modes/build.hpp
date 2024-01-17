#pragma once
#include <iostream>
#include <optional>
#include <vector>
#include "../parse/subparser.hpp"
#include "../gpki.hpp"
namespace gpki::modes::build {
struct build_params
{
  std::string key_size = "1024";
  std::string algorithm = "rsa";
  std::string key_format = "pem";
  std::string csr_crt_format = "pem";
};
std::optional<std::vector<std::string>> get_openssl_command(Profile *profile, Entity *entity, build_params *params);
int get_entity(Profile *profile, Entity *entity, build_params *params);
int ca(Profile *profile,Entity *entity,build_params *params);
int server(Profile *profile,Entity *entity,build_params *params);
int client(Profile *profile, Entity *entity, build_params *params);
} // namespace gpki::modes::build
