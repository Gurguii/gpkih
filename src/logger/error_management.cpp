#include "error_management.hpp"

using namespace gpkih;

template <typename ...Args> void Error::set(std::string fmt, Args&& ...args){
	lasterror=std::move(fmt::format(fmt,std::forward<Args>(args)...));
}

void Error::what(){
	PERROR(lasterror);
}