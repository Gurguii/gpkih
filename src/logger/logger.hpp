#pragma once
#include "../gpki.hpp" // custom typenames, fmt
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

	static inline str log_dirname = "logs";
	static inline str log_dirpath = GPKIH_BASEDIR + log_dirname + SLASH;
	static inline str logpath = log_dirpath + SLASH + "gpkih.log"; // only one actually used

	static inline void cleanup_with_exit() { printlasterror(); Logger::wait(); exit(0); };
	 
	static const Logger&get();

	template<typename ...T> static void add(Level level, strview fmt, T&& ...args);
	
	~Logger();

private:
	Logger();
	static bool start(); // called by CONSTRUCTOR() - opens log file and loads configuration from gpkih.conf
	static void wait();  // called by DESTRUCTOR() - waits for every task in `std::vector<void> Logger::tasks` to finish

	static inline std::queue<str> message_queue;        
	static inline std::mutex message_queue_mutex;

	static std::ifstream logfile;			     // set by start() using LOGPATH
	static inline std::mutex logfile_mutex;
	
	static inline int current_lines;					 // set by start() using gpkih.conf | current lines in log file
	static inline int max_lines;						 // set by start() using gpkih.conf[logs][max_lines] | maximum lines allowed in logfile
	static inline std::mutex maxlines_mutex;

	static inline Level included_levels;				 // set by start() using gpkih.conf | log levels that will be logged
	static inline std::mutex included_levels_mutex;

	static inline FormatField included_format_fields;	 // set by start() using gpkih.conf | log message fields that will be included | log syntax [date][level][msg]
	static inline std::mutex included_format_fields_mutex;

	static inline std::vector<std::future<void>> tasks{}; // holds tasks added by Logger::add()
	static inline std::mutex tasks_mutex;

	static str format_msg(Level &level, str &msg); // 

}; // class Logger

	inline Logger::FormatField operator|(Logger::FormatField lo, Logger::FormatField ro){
		return static_cast<Logger::FormatField>(static_cast<ui16>(lo) | static_cast<ui16>(ro)); 
	}
	inline bool operator&(Logger::FormatField lo, Logger::FormatField ro){
		return static_cast<bool>(static_cast<ui16>(lo) & static_cast<ui16>(ro));
	}
	inline Logger::Level operator|(Logger::Level lo, Logger::Level ro){
		return static_cast<Logger::Level>(static_cast<ui16>(lo) | static_cast<ui16>(ro));
	}
	inline bool operator&(Logger::Level lo, Logger::Level ro){
		return static_cast<bool>(static_cast<ui16>(lo) & static_cast<ui16>(ro));
	}
	

	template <typename ...T> static inline void __add_log(gpkih::Logger::Level level, str &&msg) {
		PINFO("adding log '{}'\n", msg);
	};

	template<typename ...T>
	void inline Logger::add(Level level, strview fmt, T && ...args)
	{
		switch (level) {
			case L_INFO:
				__add_log(L_INFO, std::move(fmt::format(fmt, std::forward<T>(args)...)));
				break;
			case L_WARN:
				__add_log(L_WARN, std::move(fmt::format(fmt, std::forward<T>(args)...)));
				break;
			case L_ERROR:
				__add_log(L_ERROR, std::move(fmt::format(fmt, std::forward<T>(args)...)));
				break;
		}
	}

} // namespace gpkih



