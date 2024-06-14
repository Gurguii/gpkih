#pragma once
#include <fstream> // std::ifstream, std::ofstream
#include "log_enums.hpp"

class Logger
{
	private:
		static inline std::filesystem::path basedir;
		
		std::filesystem::path logpath;
		
		std::ofstream logstream;
		std::ofstream ppstream;

		size_t maxSize     = 0;         // max log size
		size_t currentSize = 0;         // file size in bytes
		std::filesystem::path ppfile;
		logMetadata metadata;
		logMsgField includedFields = logMsgField::none;
		logLevel includedLevels = logLevel::none;
	public:
		static void setBaseDir(std::string &&basedir);
		static std::string getBaseDir();

		Logger(std::string &&filename);
		int addLog(logLevel level, std::string_view msg);
		const logMsgField& ffields();

		~Logger();
}; // class Logger