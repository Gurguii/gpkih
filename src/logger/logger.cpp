#include "logger.hpp"
#include "../config/config_management.hpp" // Config::get()

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
	// Save current lines to a file
	std::ofstream(this->linefile).write(reinterpret_cast<const char*>(&this->current_lines),sizeof(decltype(this->current_lines)));
} // Logger::~Logger

Logger::Logger(std::string &&filename){
	PDEBUG(1, "Logger::Logger()");

	this->logpath = Logger::basedir / filename += ".log";
	this->linefile = Logger::basedir / filename += ".line";

	PDEBUG(1,"logpath: {}", logpath.string());

	/* BEG - file checking */
	if (fs::exists(Logger::basedir) == false) {
		PDEBUG(3,"creating log directory '{}'\n", Logger::basedir.string());
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
		// create line counting file
		std::ofstream(this->linefile, std::ios::binary).write(reinterpret_cast<const char*>(0),sizeof(decltype(this->current_lines)));
		if (fs::exists(this->linefile) == false) {
			PWARN("couldn't create line counting file '{}'\n", this->linefile.string());
			return;
		}
	}else if(!fs::exists(logpath)){
		if (!std::ofstream(logpath).is_open()) {
			PWARN("couldn't create log file '{}'\n", logpath.string());
			return;
		}
	}
	else if (!fs::exists(this->linefile)) {
		std::ofstream(this->linefile).write(reinterpret_cast<const char*>(0), sizeof(decltype(this->current_lines)));
		if (fs::exists(this->linefile) == false) {
			PWARN("couldn't create line counting file '{}'\n", this->linefile.string());
			return;
		}
		this->current_lines = 0;
	}
	else {
		// Load current lines
		std::ifstream file(this->linefile, std::ios::binary);

		if (!file.is_open()) {
			PWARN("couldn't open line tracking file '{}'\n", this->linefile.string());
			if (std::filesystem::exists(this->linefile)) {
				PWARN("but it exists tho\n");
			}
			return;
		}

		file.read(reinterpret_cast<char*>(&this->current_lines), sizeof(decltype(this->current_lines)));
	}
	/* END - File checking */

	// set max_lines
	std::string_view msize = Config::get("logs", "max_size");
	
	size_t number = std::stoull(&msize[0],NULL,10);
	char unit = tolower(msize[msize.size()-1]);

	switch(unit){
	case 'g':
		this->max_size = static_cast<size_t>(number * 1024 * 1024 * 1024);
		break;
	case 'm':
		this->max_size = static_cast<size_t>(number * 1024 * 1024);
		break;
	case 'k':
		this->max_size = static_cast<size_t>(number * 1024);
		break;
	case 'b':
		this->max_size = static_cast<size_t>(number);
		break;
	default:
		// assume bytes
		PWARN("invalid unit in logs.max_size '{}'\n", unit);
		break;
	} 

	// set included_format_fields
	this->included_format_fields = FormatField::NONE;
	str token;
	sstream ss(Config::get("logs", "included_format_fields").data());
	while (getline(ss, token, ':')) {
		if (__str_ffield_map.find(token) != __str_ffield_map.end()) {
			// it exists, add field
			included_format_fields = included_format_fields | __str_ffield_map[token];
		}
	}

	token.assign("");
	ss.clear();

	// set include_levels
	this->included_levels = L_NONE;

	ss = std::move(sstream(Config::get("logs", "included_levels").data()));
	while (getline(ss, token, ':')) {
		if (__str_level_map.find(token) != __str_level_map.end()) {
			// it exists, add field
			included_levels = included_levels | __str_level_map[token];
		}
	}
	
	return;
} // Logger::Logger()

const FormatField& Logger::ffields(){
	return this->included_format_fields;
}

Logger *gpkih_logger = nullptr; 