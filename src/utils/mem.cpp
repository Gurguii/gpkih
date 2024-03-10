#include "utils.hpp"

using namespace gpkih;

const char* utils::mem::reallocate(std::string&& temp, utils::mem::memblock &ref) {
	if (temp.empty()) {
		return nullptr;
	}

	char* buff = (char*)malloc(temp.size() + 1);

	if (buff == nullptr) {
		return nullptr;
	}

	memcpy(buff, &temp[0], temp.size());
	buff[temp.size()] = '\0';

	ref.addresses.push(buff);

	return buff;
}