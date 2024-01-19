#pragma once
#include <string>
#include <vector>
#include <iostream>
#include "../modes/init.hpp"
#include "../modes/build.hpp"
#include "../modes/revoke.hpp"
#include "../gpki.hpp"
#include "../help/help.hpp"

namespace gpki::subparsers
{
  int init(std::vector<std::string> opts);
  int build(std::vector<std::string> opts);
  int revoke(std::vector<std::string> opts);
}
