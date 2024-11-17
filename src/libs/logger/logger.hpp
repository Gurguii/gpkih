#pragma once
#include "enums.hpp"
#include <fstream> // std::ifstream, std::ofstream

namespace gurgui::logging
{

class LogException : public std::exception{
private:
	const char *msg;
public:
	LogException() = default;
	LogException(const char *msg);
	const char *what() const noexcept override;
};

class Logger
{
	private:
		static inline std::string basedir;
		std::string logpath;
		std::ofstream logstream;

		size_t maxSize     = 0;         // max log size

		std::string mdataPath;
		logMetadata metadata;

		logMsgField includedFields;
		logLevel includedLevels;

		std::string lastError;

	public:
		/// @brief set up Logger root directory, where all log instances will be located
		/// @param basedir absolute path to directory
		/// @return 0(L_OK) if success, else returns L_PATH_NOT_ABSOLUTE or L_CANT_CREATE_FILE indicating the error
		static int setBaseDir(std::string &&basedir);
		
		/// @brief get Logger class' basedir 
		static const std::string& getBaseDir();
		Logger(std::string &&filename, size_t maxSizeBytes, logMsgField includedFields = logMsgField::all, logLevel includedLevels = logLevel::all);

		/// @brief add a new log
		/// @param level logLevel enum indicating log level, LL_INFO, LL_WARN, LL_ERROR
		/// @param msg log content
		/// @return LoggerRCode(int) indicating the error type, the error string can be retrieved with the getLastError() member method 
		int addLog(logLevel level, std::string_view msg);
		
		/* Not implemented, got to refactorize part of addLog() to avoid duplicating a lot of code*/
		//int logInfo(std::string_view msg);
		//int logWarn(std::string_view msg);
		//int logErr(std::string_view msg);

		const logMsgField& getFields();
		const logMetadata& getMetadata();
		const logLevel& getLevel();
		const std::string& getLastError();

		~Logger();
}; // class Logger

}; // namespace gurgui::logging