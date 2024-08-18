#pragma once

#include <vector>
#include <string>

namespace gpkih::parsers 
{
/// @brief does global parsing
extern int parseGlobals(std::vector<std::string> &opts);
/// @brief does generic parsing and calls specialized parser
extern int parse(std::vector<std::string> &opts);
} // namespace gpkih::parsers