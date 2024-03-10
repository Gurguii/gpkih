#include "utils.hpp"

using namespace gpkih;

bool utils::str::compare_views(std::string_view &s0, std::string_view &s1, size_t nchars){
	for(int i = 0; i < nchars; ++i){
		if(s0[i] != s1[i]){
			return false;
		}
	}
	return true;
};