#include "buffer.hpp"
#include <cstring> // memcpy

GpkihBuffer::GpkihBuffer(size_t bytes):gurgui::memory::Buffer(bytes){};

const char *GpkihBuffer::allocate_and_copy(const char *&st, size_t *length, std::string_view src){

	if(length == nullptr){
		// Don't set/use length 
		size_t _len = src.size();
		st = (char*)allocate(_len + 1);
		memcpy(const_cast<char*>(st), src.data(), _len);
	}else{
		// Set/use length
		*length = src.size();
		st = (char*)allocate((*length) + 1);
		memcpy(const_cast<char*>(st), src.data(), *length);	
	}

	return st;
}