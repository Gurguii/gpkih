#include "logger.hpp"
#include <filesystem>
#include <unordered_map>
#include <ctime>
#include <fmt/format.h>
#include <fmt/chrono.h>

#ifdef _WIN32
constexpr char SLASH = '\\';
#else
constexpr char SLASH = '/';
#endif

constexpr char EOL = '\n';

using namespace gurgui::logging;

LogException::LogException(const char *msg)
:msg(msg){};
const char *LogException::what() const noexcept{
	return msg;
}

int Logger::setBaseDir(std::string &&bdir){
	if(std::filesystem::path(bdir).is_absolute() == false){
		return L_PATH_NOT_ABSOLUTE;
	}
	if(std::filesystem::exists(bdir) == false){
		if(std::filesystem::create_directories(bdir) == false){
			return L_CANT_CREATE_FILE;
		};
	}
	Logger::basedir = std::move(bdir);
	return L_OK;
};

const std::string &Logger::getBaseDir(){
	return Logger::basedir;
}

const logMsgField& Logger::getFields(){
	return includedFields;
}

const logLevel& Logger::getLevel(){
	return includedLevels;
}

const logMetadata& Logger::getMetadata(){
	return metadata;
}

Logger::~Logger() {
	std::ofstream mdata(mdataPath,std::ios::binary);

	if(mdata.is_open() == true){
		mdata.write(reinterpret_cast<const char *>(&metadata), sizeof(metadata));
		mdata.close();
	}else{
		fwrite("WHAT THE FUCK DO I DO IN THIS CASE?\n",36,0,stdout);
	}

	logstream.flush();
	logstream.close();
} // Logger::~Logger


Logger::Logger(std::string &&filename, size_t maxSizeBytes, logMsgField iFields, logLevel iLevels)
:maxSize(maxSizeBytes),includedFields(iFields),includedLevels(iLevels),logpath(std::string{Logger::basedir}+SLASH+filename+=".log"),mdataPath(std::string{Logger::basedir}+SLASH+filename+".data"){
	// create/open log file
	if(basedir.empty()){
		throw LogException("Logger basedir hasn't been set");
	}

	if(std::filesystem::path(logpath).is_absolute() == false){
		throw LogException(logpath.c_str());
		return;
	}
	logstream = std::ofstream(logpath, std::ios::app);

	if (logstream.is_open() == false) {
		throw LogException("Couldn't open log file");
		return;
	}else if(std::filesystem::exists(logpath) == false){
		if (std::ofstream(logpath).is_open() == false) {
			throw LogException("Couldn't create log file");
			return;
		}
	}
	
	if(std::filesystem::exists(mdataPath) == true){
		std::ifstream file(mdataPath,std::ios::binary);
		if(file.is_open() == false){
			throw LogException("Couldn't open logger metadata file");
		}
		file.read(reinterpret_cast<char*>(&metadata), sizeof(metadata));
		file.close();
	}else{
		if(std::ofstream(mdataPath).is_open() == false){
			throw LogException("Couldn't create metadata file");
		}
	}
} // Logger::Logger()

std::string formatMsg(logMsgField iFields, logLevel level, std::string_view& msg){
	static std::unordered_map<logLevel, std::string> level2string{
		{LL_INFO, "info"},
		{LL_WARN, "warning"},
		{LL_ERROR,"error"},
	};

	std::ostringstream log{};
	// log syntax - [date] [level] [msg]
	iFields & LF_TIME    && log << fmt::format("{:%d-%m-%Y @ %H:%M}", std::chrono::system_clock::now());
	iFields & LF_TYPE    && log << fmt::format(" [{}] ", level2string[level]);
	iFields & LF_CONTENT && log << msg;

	return std::move(log.str());
} // Logger::format_msg();

int Logger::addLog(logLevel level, std::string_view msg){
	if(!(includedLevels & level)){
		return L_LEVEL_NOT_INCLUDED;
	};

	auto fmtLog = formatMsg(includedFields, level, msg);
	size_t size = fmtLog.size();

	while(maxSize < size + metadata.currentSize){
		--size;
	}

	if(maxSize == size + metadata.currentSize){
		// Adding this log will reach max size 
		// note: the whole log message might not fit
		logstream.write(&fmtLog[0], size);
		lastError="Maximum log size reached";
		return L_MAX_LOG_REACHED;
	}

	if(logstream.is_open() == false) {
		lastError="Couldn't open log file";
		return L_FAIL;
	}
	
	logstream.write(&fmtLog[0], size);
	logstream << EOL;

	// Update metadata info
	   level & LL_INFO  && ++metadata.infoCount 
	|| level & LL_ERROR && ++metadata.errCount 
	|| level & LL_WARN  && ++metadata.warnCount;
	
	metadata.currentSize+=size;
	++metadata.entries;

	return L_OK;
};