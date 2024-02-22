#pragma once
#include <fmt/format.h>
#include "../printing.hpp"

#include <string_view>
#include <queue>
  
#include <future> // std::async(), std::future<>

namespace gpkih
{
class Logger
{
private:
	static inline std::vector<int (*)()> cleanup_functions{};
	static std::vector<std::string_view> currently_used_files();
	std::queue<std::string> message_queue;
	std::string file;
public:
	static int cleanup_with_exit();
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