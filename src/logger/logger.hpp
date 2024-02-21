#pragma once

#include <fmt/format.h>
#include "../printing.hpp"

#include <string_view>
#include <queue>

namespace gpkih
{
class Logger
{
private:
	static std::vector<std::string_view> currently_used_files();
	std::queue<std::string> message_queue;
	std::string file;
public:
	enum class Level{
		_INFO,
		_WARNING,
		_ERROR
	};
	// won't log messages to file unless .set_logfile() is called
	Logger(); 	

	// will log messages to instance.file
	Logger(std::string log_path); 

	void set_logfile(std::string_view log_path);
	
	void add(std::string_view msg, Level lvl);
};
} // namespace gpkih