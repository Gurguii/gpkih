#pragma once
#include <fmt/format.h>
#include <fmt/core.h>

namespace gpkih::help {

extern void usage();
extern void usage_brief();
extern int callHelper(std::string_view gpkihAction);

}