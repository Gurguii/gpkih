#pragma once
#include "../printing/printing.hpp"
#include <future>  // std::vector<std::future<void>> tasks
#include <queue>   // 
#include <fstream> // std::ifstream, std::ofstream
#include <sstream> // std::stringstream, getline()

#include <filesystem>

namespace gpkih
{
	enum class Level
	{
		_NONE = 0,
		_INFO,
		_WARNING,
		_ERROR,
	};

	constexpr static inline Level L_NONE = Level::_NONE;
	constexpr static inline Level L_WARN = Level::_WARNING;
	constexpr static inline Level L_INFO = Level::_INFO;
	constexpr static inline Level L_ERROR = Level::_ERROR;

	enum class FormatField : uint16_t
	{
		NONE = 0,
		msg_type = 2,
		msg_content = 4,
		msg_time = 8,
		ALL = 15
	};

	static inline FormatField operator|(FormatField lo, FormatField ro) {
		return static_cast<FormatField>(static_cast<uint16_t>(lo) | static_cast<uint16_t>(ro));
	} // FormatField operator |

	static inline bool operator&(FormatField lo, FormatField ro) {
		return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
	} // FormatField operator &

	static inline Level operator|(Level lo, Level ro) {
		return static_cast<Level>(static_cast<uint16_t>(lo) | static_cast<uint16_t>(ro));
	} // Level operator |

	static inline bool operator&(Level lo, Level ro) {
		return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
	} // Level operator &

	/* Level -> std::string map */
	static std::unordered_map<Level, std::string> __level_str_map
	{
		{L_INFO, "info"},
		{L_WARN, "warning"},
		{L_ERROR, "error"},
	};

	static inline std::string format_msg(Level& level, const FormatField& fields, std::string&& msg) {
		std::stringstream log;
		// log syntax - [date] [level] [msg]
		fields& FormatField::msg_time&& log << std::move(fmt::format("[{:%d %h %Y @ %H:%M}] ", std::chrono::system_clock::now()));
		fields& FormatField::msg_type&& log << std::move(fmt::format("[{}] ", __level_str_map[level]));
		fields& FormatField::msg_content&& log << msg << '\n';
		return std::move(log.str());
	} // Logger::format_msg();

	class Logger
	{
	private:
		static inline std::filesystem::path basedir = "";
		
		std::filesystem::path logpath;
		std::filesystem::path linefile;

		size_t current_lines = 0;
		size_t max_size = 0;

		FormatField included_format_fields;
		Level included_levels;

		std::vector<std::future<void>> log_tasks{};

		//inline void __add_log(Logger& obj, Level level, std::string&& msg) {
		//	auto formatted = format_msg(level, obj.ffields(), msg);
		//	PWARN(formatted);
		//};

		template <typename ...T> inline void __add_log(Level level, const char *fmt, T&& ...args) {
			if(this->max_size <= std::filesystem::file_size(logpath)){
				PWARN("log reached max size {} bytes\n", max_size);
				// TODO - free old lines to keep adding
				return;
			}

			auto formatted = format_msg(level, this->ffields(), fmt::format(fmt, std::forward<T>(args)...));
			std::ofstream file(this->logpath, std::ios::app);

			if (!file.is_open()) {
				return;
			}
			
			file << std::move(formatted);
			file.close();
			++this->current_lines;
		}

	public:
		const FormatField& ffields();

		~Logger();
		// @brief constructs new Logger instance
		// @param logpath sets the name of the logging file
		Logger(std::string &&filename);

		// @brief sets Logger::basedir - directory where every logger will store its' logs
		// @param basedir path to set
		static void set_basedir(std::string &&basedir);

		// @brief returns a copy of Logger::basedir
		static std::string get_basedir();

		// @brief adds a log to file
		// @param level Level enum indicating log level (L_INFO,L_WARN,L_ERROR)
		// @param fmt fmt string that will be formatted
		// @param ...args arguments to use on `fmt` to format the string
		template<typename ...T>
		inline void add(const Level level, const char* fmt, T && ...args)
		{
			switch (level) {
			case L_INFO:
				__add_log(L_INFO, fmt, args...);
				break;
			case L_WARN:
				__add_log(L_WARN, fmt, args...);
				break;
			case L_ERROR:
				__add_log(L_ERROR, fmt, args...);
				break;
			default: 
				__add_log(L_ERROR, "invalid Level type on call to add");
				break;
			}
		}	
	}; // class Logger
} // namespace gpkih

extern gpkih::Logger *gpkih_logger;
#define ADD_LOG gpkih_logger->add