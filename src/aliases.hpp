#pragma once

#include <filesystem>
#include <cstdint>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;
namespace chrono = std::chrono;

using ui8 = uint8_t;
using ui16 = uint16_t;
using ui32 = uint32_t;
using ui64 = uint64_t;

using st_view = std::string_view;
using string = std::string;
using sstream = std::stringstream;

using iVec = std::vector<int>;
using cVec = std::vector<char>;
using sVec = std::vector<string>;
using csVec = std::vector<const char *>;