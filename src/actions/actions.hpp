#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ostream>
#include <algorithm>
#include <cstdint>

#include "../gpki.hpp"
#include "../db/database.hpp"
#include "../utils/gpkih_util_funcs.hpp"
#include "../printing.hpp"

namespace gpki::actions
{
  int init(subopts::init &params);
  int build(subopts::build &params);
  int revoke(subopts::revoke &params);
  int gencrl(subopts::gencrl &params);
  int list(subopts::list &params);
  int create_pack(subopts::create_pack &params);
  // remove is usable but not added here yet
}