#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <fmt/format.h>

namespace gpkih::utils::env
{
	extern std::string get_environment_variable(std::string_view varname);
}

namespace gpkih::utils::str
{
	extern size_t length(const char *st);
	#define len gpkih::utils::str::length
	
	extern size_t slength(const char *st);
	#define slen gpkih::utils::str::slength

	extern bool compare_views(std::string_view &s0, std::string_view &s1, size_t nchars);
	extern bool compare_views(std::string_view s0, std::string_view s1, size_t nchars);
	#define COMPARE_VIEWS gpkih::utils::str::compare_views

	extern char *replace_if(char *st, size_t nchars, char find, char replace);

	// @brief checks if buff contains any char from badcharlist
	// @return true | false indicating badchar presence
	extern bool has_badchar(std::string& buff, std::string &badcharlist);
	#define HAS_BADCHAR gpkih::utils::str::has_badchar

	// @brief removes any badchars from buff effectively calling std::string::erase()
	extern bool remove_badchar(std::string& buff, std::string &badcharlist);
	#define REMOVE_BADCHAR gpkih::utils::str::remove_badchar
}

namespace gpkih::utils::fs
{
	template <typename T> bool is_absolute_path(T path){
		#ifdef _WIN32
		  return std::isalpha(path[0]);
		#else
		  return (path[0] == '/');
		#endif
	};
	#define IS_ABSOLUTE_PATH gpkih::utils::fs::is_absolute_path
}

namespace gpkih::utils::openssl
{
	extern void genkey();

	// @brief generates dhparam with `size` bits to path `path`
	// @param path output path
	// @param size n bits in order to create dhparam
	extern int create_dhparam(std::string_view outpath, size_t size = 1024);
}