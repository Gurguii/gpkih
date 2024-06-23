#include "../mnck.hpp"

constexpr size_t gmn = 0x5f67706b69685f;

bool gpkih::mnck::dump(std::ofstream &file, size_t count){
	file.write(reinterpret_cast<const char*>(&gmn), sizeof(decltype(gmn)));
	file.write(reinterpret_cast<const char*>(&count), sizeof(decltype(count)));
	file.write(":",1);
	return true;
}

bool gpkih::mnck::check(std::ifstream &file, size_t &count){
	size_t _checker = 0;
	file.read(reinterpret_cast<char*>(&_checker), sizeof(decltype(_checker)));
	if(_checker != gmn){
		return false;
	}
	file.read(reinterpret_cast<char*>(&count), sizeof(decltype(count)));
	auto r = file.get();
	return r == ':';
}