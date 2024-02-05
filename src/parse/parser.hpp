#pragma once
#include <filesystem>
#include <string>
#include <vector>
#include <iostream>
#include <cctype>
#include <algorithm>

#include "../actions/actions.hpp"
#include "../help/help.hpp"
#include "../structs.hpp"
#include "../printing.hpp"

namespace gpki::parsers
{
  // main parser
  static int parse(int argc, const char **_args);
  // action parsers
  static int init(std::vector<str> opts);
  static int build(std::vector<str> opts);
  static int revoke(std::vector<str> opts);
  static int gencrl(std::vector<str> opts);
  static int list(std::vector<str> opts);
  static int remove(std::vector<str> opts);
  static int remove_all(std::vector<str> opts);
  static int create_pack(std::vector<str> opts);
  // unimplemented
  static int get(std::vector<str> opts);
  static int set(std::vector<str> opts);
}

static inline std::unordered_map<str, int (*)(std::vector<str>)>
    ACTION_PARSERS{
        {"init",gpki::parsers::init},
        {"list",gpki::parsers::list},
        {"remove-all",gpki::parsers::remove_all},
        {"build", gpki::parsers::build},
        {"revoke", gpki::parsers::revoke},
        {"gencrl", gpki::parsers::gencrl},
        {"remove",gpki::parsers::remove},
        {"create-pack",gpki::parsers::create_pack},
        {"get",gpki::parsers::get},
        {"set",gpki::parsers::set}
};