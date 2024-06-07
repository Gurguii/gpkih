#pragma once
#include "../gpkih.hpp" // bool DRY_RUN
#include <fstream> // std::ifstream, std::ofstream
#include <sstream> // std::stringstream, getline()
#include <unordered_map> // std::unordered_map
#include <fmt/format.h>

namespace gpkih
{
	enum class Level
	{
		NONE = 0,
		info = 2,
		warning = 4,
		error = 8,
		ALL = 15
	};
	
	constexpr static inline Level L_NONE = Level::NONE;
	constexpr static inline Level L_INFO = Level::info;
	constexpr static inline Level L_WARN = Level::warning;
	constexpr static inline Level L_ERROR = Level::error;

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
		fields & FormatField::msg_time    && log << fmt::format("[{:%d %h %Y @ %H:%M}] ", std::chrono::system_clock::now());
		fields & FormatField::msg_type    && log << fmt::format("[{}] ", __level_str_map[level]);
		fields & FormatField::msg_content && log << msg << '\n';
		return std::move(log.str());
	} // Logger::format_msg();

	class Logger
	{
	private:
		static inline std::filesystem::path basedir = "";
		
		std::filesystem::path logpath;
		std::filesystem::path ppfile;
		std::ofstream logstream;
		std::ofstream ppstream;

		size_t msize = 0;         // max log size
		size_t csize = 0;         // file size in bytes

		FormatField includedFormatFields;
		Level includedLogLevels;

		bool _ok = false;

		template <typename ...T> inline void __add_log(Level level, const char *fmt, T&& ...args) {
			auto formatted = format_msg(level, includedFormatFields, fmt::format(fmt, std::forward<T>(args)...));
			size_t size = formatted.size();

			while(this->msize < this->csize + size){
				--size;
			}

			if(this->msize == this->csize + size){
				// Adding this log will reach max size 
				// note: the whole log message might not fit
				logstream.write(&formatted[0], size);
				fprintf(stderr, "Maximum log size '%lu' reached not adding any more logs\n", this->msize);
				return;
			}

			if(logstream.is_open() == false) {
				fprintf(stderr, "Can't open log file '%s'\n", this->logpath.c_str());
				return;
			}
			
			logstream.write(&formatted[0], size);

			this->csize+=size;
		}

	public:
		const FormatField& ffields();

		~Logger();

		/// @brief constructs new Logger instance
		/// @param logpath sets the name of the logging file
		Logger(std::string &&filename);

		/// @brief check Logger instance construction status
		/// @return true if construction went well, false otherwise
		bool ok();

		/// @brief sets Logger::basedir - directory where every logger instance will store its' logs
		/// @param basedir path to set
		static void set_basedir(std::string &&basedir);

		/// @brief returns a copy of Logger::basedir
		static std::string get_basedir();

		/// @brief adds a log to file
		/// @param level Level enum indicating log level (L_INFO,L_WARN,L_ERROR)
		/// @param fmt fmt string that will be formatted
		/// @param args arguments to use on `fmt` to format the string
		template<typename ...T>
		inline void add(const Level level, const char* fmt, T && ...args)
		{
			if(DRY_RUN == true){
				return;
			}

			if(!(includedLogLevels & level)){
				return;
			}

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