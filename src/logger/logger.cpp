#include "logger.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <unordered_map>

using namespace gpkih;
Logger::Logger(std::string filepath):file(filepath){
	if(!std::filesystem::exists(file)){
		// Try to create the file
		std::ofstream f(file);
		if(!f.is_open()){
			throw std::exception();
		}
		// file created
	}
}

void Logger::add(std::string_view msg, Level level){
	
}