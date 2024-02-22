#include <iostream>
#include <fmt/format.h>
#include <unordered_map>
#include <utility>
#include <sstream>
#include <vector>
#include <tuple>

struct Profile{
	const char *name;
	const char *source;
};

struct Entity{
	const char *cn;
	const char *serial;
};

class Label{
public:
	enum class header_allignment{};
	enum class column_allignment{};
	Label(Profile prof);
	Label(Entity entity);
};

enum class ALLIGNMENT : char
{
	left = '<',
	#define L_ALLIGN ALLIGNMENT::left
	center = '^',
	#define C_ALLIGN ALLIGNMENT::center
	right = '>',
	#define R_ALLIGN ALLIGNMENT::right
};

struct FormatInfo{
	int width = 10;
	ALLIGNMENT allignment = C_ALLIGN;
};

template <typename ...T> void foo(FormatInfo &info, T&& ...args){
	std::stringstream format_string{};
	for(int i = 0; i < sizeof...(args); ++i){
		format_string << "{:" << static_cast<char>(info.allignment) << std::move(fmt::format("{}",info.width)) << "}";
	}
	fmt::print(format_string.str(), std::forward<T>(args)...);
	std::cout << "\n";
}

int main()
{
	FormatInfo info_0{.width=20,.allignment = L_ALLIGN};
	FormatInfo info_1{.width=40,.allignment = R_ALLIGN};
	
	std::string s0 = "gurgui", s1 = "maria", s2 = "juana";
	
	foo(info_0,s0,s1,s2,4);	
	foo(info_1,s0,s1,s2,4);
	return 0;
}