#include "logger.hpp"
#include "../config/config_management.hpp" // Config::get()
#include "../printing/printing.hpp"

using namespace gpkih;

Logger *program_logger = NULL;

/* std::string -> Level map */
static std::unordered_map<str,Level> __str_level_map
{
	{"info", L_INFO},
	{"warning", L_WARN},
	{"error", L_ERROR},
};

/* std::string -> FormatField map */
static std::unordered_map<str, FormatField> __str_ffield_map
{
	{"type", FormatField::msg_type},
	{"time", FormatField::msg_time},
	{"content", FormatField::msg_content}
};

void Logger::set_basedir(std::string &&bdir){
	Logger::basedir = fs::path(bdir);
};

std::string Logger::get_basedir(){
	return Logger::basedir;
}

Logger::~Logger() {
	logstream.close();
} // Logger::~Logger

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

	if(this->logstream.is_open() == false){
		_ok = false;
	}
	
	/* END - File checking */
	
	this->csize = fs::file_size(logpath);
	
	// set max_size
	std::string_view msizestr = Config::get("logs", "max_size");
	if(msizestr.empty()){
		this->msize = 0;
		return; 		
	}

	size_t number = std::stoull(&msizestr[0],NULL,10);
	char unit = tolower(msizestr[msizestr.size()-1]);

	switch(unit){
	case 'g':
		this->msize = static_cast<size_t>(number * 1024 * 1024 * 1024);
		break;
	case 'm':
		this->msize = static_cast<size_t>(number * 1024 * 1024);
		break;
	case 'k':
		this->msize = static_cast<size_t>(number * 1024);
		break;
	case 'b':
		this->msize = static_cast<size_t>(number);
		break;
	default:
		// assume bytes
		PWARN("invalid unit in logs.max_size '{}'\n", unit);
		break;
	} 

	// set includedFormatFields
	this->includedFormatFields = FormatField::NONE;
	str token;
	sstream ss(Config::get("logs", "includedFormatFields").data());
	while (getline(ss, token, ':')) {
		if (__str_ffield_map.find(token) != __str_ffield_map.end()) {
			// it exists, add field
			includedFormatFields = includedFormatFields | __str_ffield_map[token];
		}
	}

	token.assign("");
	ss.clear();

	// set include_levels
	this->includedLogLevels = L_NONE;

	ss = std::move(sstream(Config::get("logs", "includedLogLevels").data()));
	while (getline(ss, token, ':')) {
		if (__str_level_map.find(token) != __str_level_map.end()) {
			// it exists, add field
			includedLogLevels = includedLogLevels | __str_level_map[token];
		}
	}
	
	return;
} // Logger::Logger()

bool Logger::ok(){
	return _ok;
}

const FormatField& Logger::ffields(){
	return this->includedFormatFields;
}

Logger *gpkih_logger = nullptr; 