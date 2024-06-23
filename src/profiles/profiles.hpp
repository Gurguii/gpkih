#pragma once
#include <fmt/chrono.h>
#include <string>
#include "structs.hpp"

namespace gpkih::profile
{
  extern std::string caCertificatePath(Profile &ref);
  extern std::string gopensslPath(Profile &ref);
  extern std::string keyDir(Profile &ref);
  extern std::string reqDir(Profile &ref);
  extern std::string crtDir(Profile &ref);
  extern std::string crlDir(Profile &ref);
} // namespace gpkih::profile