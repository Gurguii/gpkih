#pragma once
#include "../gpki.hpp" // typename aliases
#include "../structs.hpp" // struct Profile

namespace gpkih
{
// Static class for error management
class Error {
private:
	static inline str lasterror{};
public:
	template<typename ...Args> static void set(std::string fmt, Args&& ...args);
	static void what(); // prints last error
};
} // namespace gpkih