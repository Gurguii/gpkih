#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <ostream>
#include <algorithm>

#include "../gpki.hpp"
#include "../db/database.hpp"
#include "../customFunctions/gsed.cpp"

namespace gpki::actions
{
  int init(subopts::init *params);
  int build(Profile *profile, subopts::build *params, ENTITY_TYPE type);
  int revoke(Profile *profile, subopts::revoke *params);
  int gencrl(Profile *profile, subopts::gencrl *params);
}
