#pragma once

#include <cstring>
#include <string>
#include <string_view>
#include <fmt/format.h>
#include "../printing/printing.hpp"
#include "../config/config_management.hpp"
#include "../db/entities.hpp"
#include "../memory/memmgmt.hpp"

namespace gpkih::utils::env
{
	extern std::string get_environment_variable(std::string_view varname);
}

namespace gpkih::utils::str
{
	/// @brief retrieve (does not check st == NULL) size of null terminated string
	/// @return number of characters
	extern size_t length(const char *st);
	#define len gpkih::utils::str::length
	
	/// @brief retrieve (checks if st == NULL) size of null terminated string
	/// @return number of characters or -1 if st == NULL
	extern size_t slength(const char *st);
	#define slen gpkih::utils::str::slength

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

	/// @brief generates dhparam with `size` bits to path `path`
	/// @param path output path
	/// @param size n bits in order to create dhparam: default=1024
	extern int create_dhparam(std::string_view outpath, size_t size = 1024);
}

namespace gpkih::utils::entities
{
	extern int promptForSubject(std::string_view profileName, Subject &buffer, ProfileConfig &config, EntityManager &eman);
}