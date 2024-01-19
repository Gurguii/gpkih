#pragma once
#include "../gpki.hpp"
namespace gpki::modes::revoke{
  struct DefaultParameters{
    std::string common_name;
    std::string profile_name;
    std::string reason;
  };
  int cert(Entity *entity);
  int cert(std::string profile_name, std::string common_name);
}
