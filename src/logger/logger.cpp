#include "logger.hpp"
#include "../config/config_management.hpp" // Config::get()
#include "../printing/printing.hpp"

using namespace gpkih;

Logger *program_logger = nullptr;

static const auto toBytes = [](size_t number, char unit){
	switch(unit){
		case 'g':
			return static_cast<size_t>(number * 1024 * 1024 * 1024);
		case 'm':
			return static_cast<size_t>(number * 1024 * 1024);
		case 'k':
			return static_cast<size_t>(number * 1024);
		case 'b':
			return static_cast<size_t>(number);
		default:
			// assume bytes
			return static_cast<size_t>(-1);
	} 
};

/* std::string -> Level map */
static std::unordered_map<str,Level> __str_level_map
{
	{"info", L_INFO},
	{"warning", L_WARN},
	{"error", L_ERROR},
};

/* std::string -> FormatField map */
static std::unordered_map<str, LogMsgField> __str_ffield_map
{
	{"type", LogMsgField::type},
	{"time", LogMsgField::time},
	{"content", LogMsgField::content}
};

void Logger::setBaseDir(std::string &&bdir){
	Logger::basedir = fs::path(bdir);
};

std::string Logger::getBaseDir(){
	return Logger::basedir;
}

Logger::~Logger() {
	logstream.close();
} // Logger::~Logger

static std::string format_msg(Level& level, const LogMsgField& fields, std::string&& msg) {
	static std::unordered_map<Level, std::string> level2string{
		{L_INFO, "info"},
		{L_WARN, "warning"},
		{L_ERROR,"error"},
	};
	std::ostringstream log;
	// log syntax - [date] [level] [msg]
	fields & LogMsgField::time    && log << fmt::format("[{:%d %h %Y @ %H:%M}] ", std::chrono::system_clock::now());
	fields & LogMsgField::type    && log << fmt::format("[{}] ", level2string[level]);
	fields & LogMsgField::content && log << msg << '\n';
	return std::move(log.str());
} // Logger::format_msg();

int Logger::addLog(Level level, std::string_view msg){
	size_t size = msg.size();

	while(this->msize < this->csize + size){
		--size;
	}

	if(this->msize == this->csize + size){
		// Adding this log will reach max size 
		// note: the whole log message might not fit
		logstream.write(&msg[0], size);
		fprintf(stderr, "Maximum log size '%lu' reached not adding any more logs\n", this->msize);
		return GPKIH_FAIL;
	}

	if(logstream.is_open() == false) {
		fprintf(stderr, "Can't open log file '%s'\n", this->logpath.c_str());
		return GPKIH_FAIL;
	}
	
	logstream.write(&msg[0], size);
	this->csize+=size;

	return GPKIH_OK;
};

Logger::Logger(std::string &&filename)
:logpath(Logger::basedir/filename+=".log"),ppfile(Logger::basedir/filename+=".data"){
	PDEBUG(1, "Logger::Logger()");

	/* BEG - file checking */
	if (fs::exists(Logger::basedir) == false) {
		// create log directory
		if (fs::create_directories(Logger::basedir) == false) {
			PWARN("couldn't create log directory\n");
			return;
		}
		// create log file 
		if (std::ofstream(logpath).is_open() == false) {
			PWARN("couldn't create log file '{}'\n", logpath.string());
			return;
		}
	}else if(!fs::exists(logpath)){
		if (!std::ofstream(logpath).is_open()) {
			PWARN("couldn't create log file '{}'\n", logpath.string());
			return;
		}
	}
	
	// Create streams
	this->logstream = std::ofstream(this->logpath, std::ios::app);

	/* END - File checking */
	
	this->csize = fs::file_size(logpath);
	
	// set max_size
	std::string_view msizestr = Config::get("logs", "max_size");
	if(msizestr.empty()){
		this->msize = 0;
		return; 		
	}

	size_t number = std::stoull(&msizestr[0],nullptr,10);
	char unit = tolower(msizestr[msizestr.size()-1]);

	if((this->msize = toBytes(number, unit)) == -1){
		PWARN("invalid unit in logs.max_size '{}'\n", unit);
		return;
	}

	// set includedFormatFields
	this->includedFields = LogMsgField::none;
	str token;
	sstream ss(Config::get("logs", "includedFormatFields").data());
	while (getline(ss, token, ':')) {
		if (__str_ffield_map.find(token) != __str_ffield_map.end()) {
			// it exists, add field
			includedFields = includedFields | __str_ffield_map[token];
		}
	}

	token.assign("");
	ss.clear();

	// set include_levels
	this->includedLevels = L_NONE;

	ss = std::move(sstream(Config::get("logs", "includedLevels").data()));
	while (getline(ss, token, ':')) {
		if (__str_level_map.find(token) != __str_level_map.end()) {
			// it exists, add field
			includedLevels = includedLevels | __str_level_map[token];
		}
	}
	
	return;
} // Logger::Logger()

Logger *gpkih_logger = nullptr; 