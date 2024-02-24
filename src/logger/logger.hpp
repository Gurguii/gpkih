#pragma once
#include "../gpki.hpp" // custom typenames, fmt
 
#include <future> // std::vector<std::future<void>> Logger::tasks
#include <queue> // Logger::message_queue
namespace gpkih
{

class Logger
{

public:
	enum class Level{
		NONE = 0,
#define L_NONE = Logger::Level::NONE
		_INFO,
#define L_INFO = Logger::Level::_INFO
		_WARNING,
#define L_WARN = Logger::Level::_WARNING
		_ERROR
#define L_ERROR = Logger::Level::_ERROR
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

	static inline void cleanup_with_exit() { printlasterror(); Logger::wait(); };
	 
	static Logger&const get();

	template<typename ...T>void static add_error(std::string_view fmt, T&& ...args);
	template<typename ...T>void static add_info(std::string_view fmt, T&& ...args);
	template<typename ...T>void static add_warn(std::string_view fmt, T&& ...args);
	
	~Logger();

private:
	Logger();
	static bool start(); // called by CONSTRUCTOR() - opens log file and loads configuration from gpkih.conf
	static void wait();  // called by DESTRUCTOR() - waits for every task in `std::vector<void> Logger::tasks` to finish

	template<typename ...T>void static add(Level level, std::string_view fmt, T&& ...args);

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
} // namespace gpkih