#pragma once
#include <fstream> // std::ifstream, std::ofstream
#include <filesystem>

namespace gpkih
{
	enum class Level
	{
		none = 0,
		info = 2,
		warning = 4,
		error = 8,
		all = 15
	};

	constexpr static Level L_NONE = Level::none;
	constexpr static Level L_INFO = Level::info;
	constexpr static Level L_WARN = Level::warning;
	constexpr static Level L_ERROR = Level::error;
	
	enum class LogMsgField : uint16_t
	{
		none = 0,
		type = 2,
		content = 4,
		time = 8,
		all = 15
	};
	
	class Logger
	{
		private:
			static inline std::filesystem::path basedir;
			
			std::filesystem::path logpath;
			std::filesystem::path ppfile;
			std::ofstream logstream;
			std::ofstream ppstream;
	
			size_t msize = 0;         // max log size
			size_t csize = 0;         // file size in bytes
	
			LogMsgField includedFields;
			Level includedLevels;
		public:
			static void setBaseDir(std::string &&basedir);
			static std::string getBaseDir();
	
			Logger(std::string &&filename);
			int addLog(Level level, std::string_view msg);
			const LogMsgField& ffields();
	
			~Logger();
	}; // class Logger

	static inline LogMsgField operator|(LogMsgField lo, LogMsgField ro) {
		return static_cast<LogMsgField>(static_cast<uint16_t>(lo) | static_cast<uint16_t>(ro));
	} // LogMsgField operator |
	static inline bool operator&(LogMsgField lo, LogMsgField ro) {
		return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
	} // LogMsgField operator &
	static inline Level operator|(Level lo, Level ro) {
		return static_cast<Level>(static_cast<uint16_t>(lo) | static_cast<uint16_t>(ro));
	} // Level operator |
	static inline bool operator&(Level lo, Level ro) {
		return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
	} // Level operator &
} // namespace gpkih