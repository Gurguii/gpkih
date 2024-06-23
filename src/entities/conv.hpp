#pragma once
#include "enums.hpp"
#include <string>

namespace gpkih::entity::conversion
{
  [[ nodiscard("Pointless to call this and ignore the returned value gordito") ]]
  extern std::string toString(ENTITY_TYPE eType);
  [[ nodiscard("Pointless to call this and ignore the returned value gordito") ]]
  extern std::string toString(ENTITY_STATUS eStatus);

  template <typename T> T toEnum(std::string_view enumName);
  template <>
  ENTITY_TYPE toEnum<ENTITY_TYPE>(std::string_view enumName);
  template <>
  ENTITY_STATUS toEnum<ENTITY_STATUS>(std::string_view enumName);
}