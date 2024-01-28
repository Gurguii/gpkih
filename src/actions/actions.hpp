#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ostream>
#include <algorithm>
#include <cstdint>

#include "../gpki.hpp"
#include "../db/database.hpp"
#include "../utils/gsed.cpp"
#include "../printing.hpp"

namespace gpki::actions
{
  int init(subopts::init *params);
  int build(Profile *profile, subopts::build *params, ENTITY_TYPE type);
  int revoke(Profile *profile, subopts::revoke *params);
  int gencrl(Profile *profile, subopts::gencrl *params);
  int list(subopts::list &params);
  // remove is usable but not added here yet
}