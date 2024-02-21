#include "Logger.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <unordered_map>


using namespace gpkih;

static std::unordered_map<Logger::Level,STYLE> __level_style_map
{
	{Logger::Level::ERROR, S_ERROR},
	{Logger::Level::WARNING, S_WARNING},
	{Logger::Level::INFO, S_INFO}
};

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

template<typename ...T> void Logger::add(Level level, std::string_view fmt, T&& ...args){
	message_queue.emplace(std::move(fmt::print(__level_style_map[level],fmt,std::forward<T>(args)...)));
}