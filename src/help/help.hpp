#pragma once
#include "../gpki.hpp"
#include "../printing.hpp"
#include <iostream>
#include <unordered_map>

namespace gpki::help {
void usage();
}
namespace gpki::help::build {
void usage();
}
namespace gpki::help::revoke {
void usage();
}
namespace gpki::help::gencrl {
void usage();
}
namespace gpki::help::list {
void usage();
}
namespace gpki::help::init {
void usage();
}
namespace gpki::help::remove {
void usage();
}
namespace gpki::help::create_pack {
void usage();
}
namespace gpki::help::get {
void usage();
}
namespace gpki::help::set {
void usage();
}
using namespace gpki;
void call_helper(strview action);
