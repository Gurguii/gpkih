#pragma once
#include <fmt/format.h>
#include <fmt/core.h>
#include <unordered_map>

struct Helper{
	const char *usage;
	const char *examples;
};

namespace gpkih::help {
extern void usage();
extern void usage_brief();
extern std::unordered_map<std::string, Helper> helpers;
}