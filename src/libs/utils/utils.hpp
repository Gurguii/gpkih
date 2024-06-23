#pragma once
#include <cstring>
#include <string>
#include <string_view>

namespace gpkih::utils::env
{
	extern std::string get_environment_variable(std::string_view varname);
}

namespace gpkih::utils::units
{
	constexpr const size_t toBytes(size_t number, char unit){
		switch(unit){
			case 'g':
				return static_cast<size_t>(number * 1024 * 1024 * 1024);
			case 'm':
				return static_cast<size_t>(number * 1024 * 1024);
			case 'k':
				return static_cast<size_t>(number * 1024);
			case 'b':
				return static_cast<size_t>(number);
			default:
				// assume bytes
				return static_cast<size_t>(0);
		}
	};
}

namespace gpkih::utils::str
{
	/// @brief retrieve (does not check st == nullptr) size of null terminated string
	/// @return number of characters
	extern size_t glength(const char *st);
	
	/// @brief retrieve (checks if st == nullptr) size of null terminated string
	/// @return number of characters or 0 if st == nullptr
	extern size_t gslength(const char *st);

	extern bool compareViews(std::string_view &s0, std::string_view &s1, size_t nchars);
	extern bool compareViews(std::string_view s0, std::string_view s1, size_t nchars);
	#define COMPARE_VIEWS gpkih::utils::str::compareViews

	extern char *replace_if(char *st, size_t nchars, char find, char replace);

	/// @brief checks if buff contains any char from badcharlist
	/// @return true | false indicating badchar presence
	extern bool has_badchar(std::string& buff, std::string &badcharlist);
	#define HAS_BADCHAR gpkih::utils::str::has_badchar

	/// @brief removes any badchars from buff effectively calling std::string::erase()
	extern bool remove_badchar(std::string& buff, std::string &badcharlist);
	#define REMOVE_BADCHAR gpkih::utils::str::remove_badchar
}

namespace gpkih::utils::fs
{
	extern bool is_absolute_path(std::string_view path);
	#define IS_ABSOLUTE_PATH gpkih::utils::fs::is_absolute_path
}