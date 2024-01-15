#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../modes/profile.hpp"
#include "../modes/build.hpp"
#include "../modes/entity.hpp"
#include "../gpki.hpp"


struct profile_params
{

};

struct entity_params
{

};

namespace gpki::subparsers
{
  int get_entity();
  int build(std::vector<std::string> opts);
  int profile(std::vector<std::string> opts);
  int entity(std::vector<std::string> opts);
}
