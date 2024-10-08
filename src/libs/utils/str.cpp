#include "utils.hpp"
#include <algorithm> // std::remove_if()
#include <variant>

using namespace gurgui;

bool utils::str::compareViews(std::string_view &s0, std::string_view &s1, size_t nchars){
	for(int i = 0; i < nchars; ++i){
		if(s0[i] != s1[i]){
			return false;
		}
	}
	return true;
};

bool utils::str::compareViews(std::string_view s0, std::string_view s1, size_t nchars){
	for(int i = 0; i < nchars; ++i){
		if(s0[i] != s1[i]){
			return false;
		}
	}
	return true;
};

size_t utils::str::glength(const char *st){
	size_t l = 0;
	while(st[l] != '\0'){
		++l;
	}
	return l;
}

size_t utils::str::gslength(const char *st){
	if(st == nullptr){
		return 0;
	}
	size_t l = 0;
	while(st[l] != '\0'){
		++l;
	}
	return l;
}

char *utils::str::replace_if(char *st, size_t nchars, char find, char replace){
	for(int i = 0; i < nchars; ++i){
		if(st[i] == find){
			st[i] = replace;
		}
	}
	return st;
}

bool utils::str::has_badchar(std::string& buff, std::string &badcharlist) {
    for (const char& c : buff) {
        if (badcharlist.find(c) != std::variant_npos) {
            return true;
        }
    }
    return false;
};

bool utils::str::remove_badchar(std::string& buff, std::string &badcharlist) {
	auto presize = buff.size();
    buff.erase(std::remove_if(buff.begin(), buff.end(), [&badcharlist](char& c) {return badcharlist.find(c) != -1;}), buff.end());
    return presize > buff.size();
}