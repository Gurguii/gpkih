#pragma once
#include "../gpki.hpp"
#include "../config/config_management.hpp" // Config::get()
#include <future> // std::vector<std::future<void>> Logger::tasks
#include <queue> // Logger::message_queue

namespace gpkih
{

class Logger
{

public:
	enum class Level{
		NONE = 0,
#define L_NONE Logger::Level::NONE
		_INFO,
#define L_INFO Logger::Level::_INFO
		_WARNING,
#define L_WARN Logger::Level::_WARNING
		_ERROR,
#define L_ERROR Logger::Level::_ERROR
	};

	enum class FormatField : ui16
	{
		NONE = 0,
		msg_type = 2,
		msg_content = 4,
		msg_time = 8,
		ALL = 15
	};

	str logpath; // only one actually used

	void cleanup_with_exit();
	 
	const Logger&get();
	const Logger* const get_ptr();

	template<typename ...T> void add(Level level, strview fmt, T&& ...args);
	
	~Logger();
	Logger(str logpath);

	str format_msg(Level &level, str &msg);
private:
	bool start(); // called by CONSTRUCTOR() - opens log file and loads configuration from gpkih.conf
	void wait();  // called by DESTRUCTOR() - waits for every task in `std::vector<void> Logger::tasks` to finish

	std::queue<str> message_queue;        
	std::mutex message_queue_mutex;
			     		 
	std::mutex logfile_mutex;
	
	int current_lines;					    // set by start() using gpkih.conf | current lines in log file
	int max_lines;						    // set by start() using gpkih.conf[logs][max_lines] | maximum lines allowed in logfile
	std::mutex maxlines_mutex;

	Level included_levels;				    // set by start() using gpkih.conf | log levels that will be logged
	std::mutex included_levels_mutex;

	FormatField included_format_fields;	    // set by start() using gpkih.conf | log message fields that will be included | log syntax [date][level][msg]
	std::mutex included_format_fields_mutex;

	std::vector<std::future<void>> tasks{}; // holds tasks added by Logger::add()
	std::mutex tasks_mutex;

	

}; // class Logger
	
	static inline Logger::FormatField operator|(Logger::FormatField lo, Logger::FormatField ro){
		return static_cast<Logger::FormatField>(static_cast<ui16>(lo) | static_cast<ui16>(ro)); 
	} // FormatField operator |

	static inline bool operator&(Logger::FormatField lo, Logger::FormatField ro){
		return static_cast<bool>(static_cast<ui16>(lo) & static_cast<ui16>(ro));
	} // FormatField operator &

	static inline Logger::Level operator|(Logger::Level lo, Logger::Level ro){
		return static_cast<Logger::Level>(static_cast<ui16>(lo) | static_cast<ui16>(ro));
	} // Level operator |

	static inline bool operator&(Logger::Level lo, Logger::Level ro){
		return static_cast<bool>(static_cast<ui16>(lo) & static_cast<ui16>(ro));
	} // Level operator &
	
	template <typename ...T>  static inline void __add_log(gpkih::Logger &obj, gpkih::Logger::Level level, str &&msg) {
		auto formatted = obj.format_msg(level, msg);
		PRINT(formatted);
	};

	template<typename ...T>
	inline void Logger::add(Level level, strview fmt, T && ...args)
	{
		switch (level) {
			case L_INFO:
				__add_log(*this, L_INFO, std::move(fmt::format(fmt, std::forward<T>(args)...)));
				break;
			case L_WARN:
				__add_log(*this, L_WARN, std::move(fmt::format(fmt, std::forward<T>(args)...)));
				break;
			case L_ERROR:
				__add_log(*this, L_ERROR, std::move(fmt::format(fmt, std::forward<T>(args)...)));
				break;
			default:
				__add_log(*this, L_ERROR, "invalid Level type on call to Logger::add");
				break;
		}
	}

} // namespace gpkih
