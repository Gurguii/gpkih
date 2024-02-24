#pragma once
#include <fmt/format.h>

#include "../printing.hpp"
#include "../gpki.hpp"

#include <string_view>
#include <queue>
#include <future>

namespace gpkih
{


class Logger
{

public:
	enum class Level{
		NONE = 0,
		_INFO,
		_WARNING,
		_ERROR
	};

	enum class FormatField : ui16
	{
		NONE = 0,
		msg_type = 2,
		msg_content = 4,
		msg_time = 8,
		ALL = 15
	};

	static inline void cleanup_with_exit(){exit(0);};

	static bool const is_ok();
	 
	// attempts opening logs
	static bool start();
	
	template<typename ...T>void add(Level level, std::string_view fmt, T&& ...args);

private:
	static inline str log_dirname = "logs";
	static inline str log_dirpath = GPKIH_BASEDIR + log_dirname + SLASH; 
	static inline str logpath = log_dirpath + SLASH + "gpkih.log"; // only one actually used
	
	static std::queue<str> message_queue;
	static inline std::mutex message_queue_mutex;

	static std::ifstream logfile;			     // set by start() using LOGPATH
	static inline std::mutex logfile_mutex;
	
	static int current_lines;					 // set by start() using gpkih.conf
	static int max_lines;						 // set by start() using gpkih.conf
	static inline std::mutex maxlines_mutex;

	static Level included_levels;				 // set by start() using gpkih.conf
	static inline std::mutex included_levels_mutex;

	static FormatField included_format_fields;	 // set by start() using gpkih.conf
	static inline std::mutex included_format_fields_mutex;
	template <typename ...T> static str format_msg(T&& ...args); // properly format msg

}; // class Logger
	inline Logger::FormatField operator|(Logger::FormatField lo, Logger::FormatField ro){
		return static_cast<Logger::FormatField>(static_cast<ui16>(lo) | static_cast<ui16>(ro)); 
	}
	inline Logger::FormatField operator&(Logger::FormatField lo, Logger::FormatField ro){
		return static_cast<Logger::FormatField>(static_cast<ui16>(lo) & static_cast<ui16>(ro));
	}
	inline Logger::Level operator|(Logger::Level lo, Logger::Level ro){
		return static_cast<Logger::Level>(static_cast<ui16>(lo) | static_cast<ui16>(ro));
	}
	inline Logger::Level operator&(Logger::Level lo, Logger::Level ro){
		return static_cast<Logger::Level>(static_cast<ui16>(lo) & static_cast<ui16>(ro));
	}
} // namespace gpkih