#pragma once
#include "enums.hpp"

namespace gpkih::config
{
	extern int syncFile(std::string_view srcConfigPath, ConfigMap *cmap);
	extern int loadFile(std::string_view src, ConfigMap &buff);
}