#pragma once
#include "../gpki.hpp"
#include <string>
#include <string_view>
#include <stack>

namespace gpkih::utils::env
{

	extern std::string get_environment_variable(std::string_view varname);
}

namespace gpkih::utils::str
{
	extern bool compare_views(std::string_view &s0, std::string_view &s1, size_t nchars);
}

namespace gpkih::utils::mem{
	struct memblock
	{
		std::stack<char*> addresses{};
		~memblock() {
			for (char* ptr = addresses.top(); ptr != NULL; addresses.pop()) {
				free(ptr);
				ptr = NULL;
			}
		}
	};
	extern const char *reallocate(std::string &&src, memblock &ref);
}