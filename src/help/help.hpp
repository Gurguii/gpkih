#pragma once
#include <iostream>
#include <unordered_map>


namespace gpki::help::generic {
void usage();
}
namespace gpki::help::build {
void usage();
}
namespace gpki::help::revoke {
void usage();
}

#define call_helper(x) (x == "build" ? gpki::help::build::usage() : (x == "revoke" ? gpki::help::revoke::usage() : gpki::help::generic::usage() ))
