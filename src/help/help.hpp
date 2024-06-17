#pragma once
#include <fmt/format.h>
#include <fmt/core.h>
#include <unordered_map>
#include "../printing/printing.hpp"

/* Interface for action helpers */

namespace gpkih::help {
extern void usage();
extern void usage_brief();
extern void call_helper(std::string_view action);
extern std::unordered_map<std::string, void(*)()> help_funcs;
}

namespace gpkih::help::build {
	extern void usage();
}
namespace gpkih::help::revoke {
	extern void usage();
}
namespace gpkih::help::gencrl {
	extern void usage();
}
namespace gpkih::help::list {
	extern void usage();
}
namespace gpkih::help::init {
	extern void usage();
}
namespace gpkih::help::remove {
	extern void usage();
}
namespace gpkih::help::get {
	extern void usage();
}
namespace gpkih::help::set {
	extern void usage();
}
namespace gpkih::help::rename{
	extern void usage();
}
namespace gpkih::help::reset{
	extern void usage();
}