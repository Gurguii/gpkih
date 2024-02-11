#pragma once
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <iostream>
#include <string>
#include <vector>

#include "../actions/actions.hpp"
#include "../help/help.hpp"
#include "../printing.hpp"
#include "../structs.hpp"

namespace gpki::parsers {
// main parser
static int parse(int argc, const char **_args);
// action parsers
static int init(std::vector<str> opts);
static int build(std::vector<str> opts);
static int revoke(std::vector<str> opts);
static int gencrl(std::vector<str> opts);
static int list(std::vector<str> opts);
static int remove(std::vector<str> opts);
// unimplemented
static int get(std::vector<str> opts);
static int set(std::vector<str> opts);
static int genkey(std::vector<str> opts);
} // namespace gpki::parsers

static inline std::unordered_map<str, int (*)(std::vector<str>)> ACTION_PARSERS{
    {"init", gpki::parsers::init},
    {"list", gpki::parsers::list},
    {"build", gpki::parsers::build},
    {"revoke", gpki::parsers::revoke},
    {"gencrl", gpki::parsers::gencrl},
    {"remove", gpki::parsers::remove},
    {"get", gpki::parsers::get},
    {"set", gpki::parsers::set},
    {"genkey", gpki::parsers::genkey}
};
