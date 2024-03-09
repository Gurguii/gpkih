#include "logger.hpp"
#include "../config/config_management.hpp" // Config::get()

using namespace gpkih;

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

Logger::~Logger() {
	// Save current lines to a file
	std::ofstream(this->linefile) << current_lines;
} // Logger::~Logger

Logger::Logger(std::string &&logdir){
	
	this->basedir = std::move(logdir);
	this->logpath = this->basedir / "gpkih.log";
	this->linefile = this->basedir / ".line";

	if (fs::exists(basedir) == false) {
		PINFO("creating log directory '{}'\n", basedir.string());
		// create log directory
		if (fs::create_directories(basedir) == false) {
			PWARN("couldn't create log directory\n");
			return;
		}
		// create log file 
		if (std::ofstream(logpath).is_open() == false) {
			PWARN("couldn't create log file '{}'\n", logpath.string());
			return;
		}
		// create line counting file
		std::ofstream(this->linefile) << "0";
		if (fs::exists(this->linefile) == false) {
			PWARN("couldn't create line counting file '{}'\n", this->linefile.string());
			return;
		}
		this->current_lines = 0;
	}else if(!fs::exists(logpath)){
		if (!std::ofstream(logpath).is_open()) {
			PWARN("couldn't create log file '{}'\n", logpath.string());
			return;
		}
	}
	else if (!fs::exists(this->linefile)) {
		std::ofstream(this->linefile) << "0";
		if (fs::exists(this->linefile) == false) {
			PWARN("couldn't create line counting file '{}'\n", this->linefile.string());
			return;
		}
		this->current_lines = 0;
	}
	else {
		// Load current lines
		std::ifstream file(this->linefile, std::ios::in);

		if (!file.is_open()) {
			PWARN("couldn't open line tracking file '{}'\n", this->linefile.string());
			if (std::filesystem::exists(this->linefile)) {
				PWARN("but it exists tho\n");
			}
			return;
		}

		std::string n;
		file >> n;

		this->current_lines = std::move(strtol(&n[0], nullptr, 10));
	}

	// set max_lines
	this->max_lines = strtol(Config::get("logs", "max_lines").data(), nullptr, 10);
	if (current_lines >= max_lines) {
		return;
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