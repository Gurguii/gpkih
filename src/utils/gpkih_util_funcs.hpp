#pragma once
#include "../gpki.hpp"
#include <fstream>
#include <filesystem>

extern int can_write(std::string path);
extern bool hasWritePermissions(std::string path);
 