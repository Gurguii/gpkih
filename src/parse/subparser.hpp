#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../actions/actions.hpp"
#include "../gpki.hpp"
#include "../help/help.hpp"

namespace gpki::subparsers
{
  int init(std::vector<std::string> opts);
  int build(Profile *profile, std::vector<std::string> opts);
  int revoke(Profile *profile, std::vector<std::string> opts);
  int gencrl(Profile *profile, std::vector<std::string> opts);
}
