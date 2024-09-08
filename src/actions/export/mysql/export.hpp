#pragma once

#include <string_view>
#include <vector>

namespace gpkih::mysql
{
	extern int exportDB(std::string_view outDir, std::vector<std::string> &args);
}