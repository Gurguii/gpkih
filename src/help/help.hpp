#pragma once
#include <fmt/format.h>
#include <fmt/core.h>
#include "../config/config_management.hpp"

using str = std::string;
using strview = std::string_view;

namespace gpkih::help {
extern void usage();
extern void usage_brief();
extern void call_helper(strview action);
extern std::unordered_map<str, void(*)()> help_funcs;
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
namespace gpkih::help::create_pack {
extern void usage();
}
namespace gpkih::help::get {
extern void usage();
}
namespace gpkih::help::set {
extern void usage();
}