#include "logger.hpp"
#include "../config/config_management.hpp"

#include <exception>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <unordered_map>
#include <sstream>

using namespace gpkih;

static std::unordered_map<Logger::Level,STYLE> __level_style_map
{
	{Logger::Level::_ERROR, S_ERROR},
	{Logger::Level::_WARNING, S_WARNING},
	{Logger::Level::_INFO, S_INFO}
};

/* std::string -> Logger::Level map */
static std::unordered_map<str,Logger::Level> __str_level_map
{
	{"info", Logger::Level::_INFO},
	{"warning", Logger::Level::_WARNING},
	{"error", Logger::Level::_ERROR},
};

/* std::string -> Logger::FormatField map */
static std::unordered_map<str, Logger::FormatField> __str_ffield_map
{
	{"type", Logger::FormatField::msg_type},
	{"time", Logger::FormatField::msg_time},
	{"content", Logger::FormatField::msg_content}
};

bool Logger::start(){
	if(!fs::exists(logpath)){
		// create log file 
		if(!std::ofstream(logpath).is_open()){
			return false;
		}
	}
	// TODO - change this 
	// set current lines
	std::ifstream file(logpath);
	str line;
	current_lines = 0;
	while(getline(file,line)){++current_lines;}
	file.close();

	// set max_lines
	max_lines = strtol(Config::get("logs","max_lines").data(),nullptr, 10);
	if(current_lines >= max_lines){
		return false;	
	}

	// set included_format_fields
	included_format_fields = Logger::FormatField::NONE;
	str token;
	sstream ss(Config::get("logs","included_format_fields").data());
	while(getline(ss, token, ':')){
		if(__str_ffield_map.find(token) != __str_ffield_map.end()){
			// it exists, add field
			included_format_fields = included_format_fields | __str_ffield_map[token];
		}
	}

	token.assign("");
	ss.clear();

	// set include_levels
	included_levels = Logger::Level::NONE;
	ss = std::move(sstream(Config::get("logs","included_levels").data()));
	while(getline(ss,token,':')){
		if(__str_level_map.find(token) != __str_level_map.end()){
			// it exists, add field
			included_levels = included_levels | __str_level_map[token];	
		}
	}

	return true;
} // Logger::start()

template<typename ...T> void Logger::add(Level level, std::string_view fmt, T&& ...args){
	// TODO - just make an asynchronous call to 
	// a method that actually handles checking everything
	// and add future to a vector that wait for every task to finish
	std::lock_guard<std::mutex> guard(logfile_mutex);
	if(!logfile.is_open()){
		seterror("logfile not opened\n");
	}
	PINFO("file opened, adding log entry\n");
	// format message appropiately
	// 
	message_queue.emplace(std::move(fmt::print(__level_style_map[level],fmt,std::forward<T>(args)...)));
}