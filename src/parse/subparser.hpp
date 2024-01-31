#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <algorithm>

#include "../actions/actions.hpp"
#include "../gpki.hpp"
#include "../help/help.hpp"
#include "../structs.hpp"
#include "../printing.hpp"

namespace gpki::subparsers
{
  int init(std::vector<str> opts);
  int build(std::vector<str> opts);
  int revoke(std::vector<str> opts);
  int gencrl(std::vector<str> opts);
  int list(std::vector<str> opts);
  int remove(std::vector<str> opts);
  int remove_all(std::vector<str> opts);
  int create_pack(std::vector<str> opts);
}
