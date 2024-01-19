#pragma once
#include "../gpki.hpp"
#include <iostream>
#include "../db/database.hpp"
#include <algorithm>
#include <unordered_map>
#include <vector>
#include "../customFunctions/gsed.cpp"
#include "build.hpp"
#include <cstring>
namespace gpki::modes::init{
  struct DefaultParameters{
    int prompt = 1;
    /* Unused yet */
    std::string profile_name;
    std::string profile_source_dir;
    int create_tls_keys = 0;
    int create_ca_cert = 0;
  };
int create_new_profile();
}
