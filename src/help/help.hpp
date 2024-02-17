#pragma once
#include "../gpki.hpp"
#include "../printing.hpp"
#include <iostream>
#include <unordered_map>

namespace gpkih::help {
void usage();
}
namespace gpkih::help::build {
void usage();
}
namespace gpkih::help::revoke {
void usage();
}
namespace gpkih::help::gencrl {
void usage();
}
namespace gpkih::help::list {
void usage();
}
namespace gpkih::help::init {
void usage();
}
namespace gpkih::help::remove {
void usage();
}
namespace gpkih::help::create_pack {
void usage();
}
namespace gpkih::help::get {
void usage();
}
namespace gpkih::help::set {
void usage();
}
using namespace gpkih;
void call_helper(strview action);
