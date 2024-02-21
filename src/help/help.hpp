#pragma once
#include "../gpki.hpp"
#include "../printing.hpp"
#include <iostream>
#include <unordered_map>

namespace gpkih::help {
void usage();
extern void call_helper(strview action);
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