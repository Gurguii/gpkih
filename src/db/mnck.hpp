#pragma once
#include <fstream>
#include <cstdint>
#include "../printing/printing.hpp"

namespace gpkih::mnck
{
	extern bool dump(std::ofstream &file, size_t count);
	extern bool check(std::ifstream &file, size_t &count);	
}