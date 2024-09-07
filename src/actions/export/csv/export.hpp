#pragma once

#include <string_view>
#include <vector>

namespace gpkih::csv
{
	extern int exportDB(std::string_view dbPath, std::vector<std::string> &args);
}