#pragma once
#include <fstream>
#include <cstdint>
#include "../printing/printing.hpp"

constexpr static inline size_t gmn = 0x5f67706b69685f;

namespace gpkih::mnck
{
	extern bool dump(std::ofstream &file, size_t count);
	extern bool check(std::ifstream &file, size_t &count);	
}