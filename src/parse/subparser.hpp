#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../modes/profile.hpp"
#include "../modes/build.hpp"
#include "../modes/entity.hpp"
namespace gpki::subparsers
{
  int build(std::vector<std::string> opts);
  int profile(std::vector<std::string> opts);
  int entity(std::vector<std::string> opts);
}
