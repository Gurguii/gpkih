#include "utils.hpp"

using namespace gpkih;
bool utils::fs::is_absolute_path(std::string_view path){
	#ifdef _WIN32
	return path[0] == '\\';
	#else
	return path[0] == '/';
	#endif
}