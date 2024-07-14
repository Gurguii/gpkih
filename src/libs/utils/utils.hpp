#pragma once
#include <cstring>
#include <string>
#include <string_view>

namespace gurgui::utils::env
{
	extern std::string get_environment_variable(std::string_view varname);
}

namespace gurgui::utils::units
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

namespace gurgui::utils::str
{
	/// @brief retrieve (does not check st == nullptr) size of null terminated string
	/// @return number of characters
	extern size_t glength(const char *st);
	
	/// @brief retrieve (checks if st == nullptr) size of null terminated string
	/// @return number of characters or 0 if st == nullptr
	extern size_t gslength(const char *st);

	/// @brief replaces ocurrences in a char array
	/// @return pointer to given charr array
	extern char *replace_if(char *st, size_t nchars, char find, char replace);

	/// @brief checks if buff contains any char from badcharlist
	/// @return true | false indicating badchar presence
	extern bool has_badchar(std::string& buff, std::string &badcharlist);

	/// @brief removes any badchars from buff effectively calling std::string::erase()
	/// @return true if badchars were removed, else false
	extern bool remove_badchar(std::string& buff, std::string &badcharlist);
}

namespace gurgui::utils::fs
{
	extern bool is_absolute_path(std::string_view path);
	#define IS_ABSOLUTE_PATH gurgui::utils::fs::is_absolute_path
}