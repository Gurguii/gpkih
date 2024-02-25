#include "logger.hpp"

#include <fstream> // std::ifstream, std::ofstream
#include <sstream> // std::stringstream, getline()

using namespace gpkih;

static std::unordered_map<Logger::Level,STYLE> __level_style_map
{
	{Logger::Level::_ERROR, S_ERROR},
	{Logger::Level::_WARNING, S_WARNING},
	{Logger::Level::_INFO, S_INFO}
};

/* std::string -> Logger::Level map */
static std::unordered_map<str,Logger::Level> __str_level_map
{
	{"info", Logger::Level::_INFO},
	{"warning", Logger::Level::_WARNING},
	{"error", Logger::Level::_ERROR},
};

/* Logger::Level -> std::string map */
static std::unordered_map<Logger::Level, std::string> __level_str_map
{
	{Logger::Level::_INFO, "info"},
	{Logger::Level::_WARNING, "warning"},
	{Logger::Level::_ERROR, "error"},
};

/* std::string -> Logger::FormatField map */
static std::unordered_map<str, Logger::FormatField> __str_ffield_map
{
	{"type", Logger::FormatField::msg_type},
	{"time", Logger::FormatField::msg_time},
	{"content", Logger::FormatField::msg_content}
};

// Logger getter - this ensures only 1 instance of Logger is created
const Logger& Logger::get() {
	// since its a static function member,
	// this is working like a coroutine
	static Logger instance{};
	return instance;
};

// Logger constructor
Logger::Logger() {
	Logger::start();
};

// Logger destructor
Logger::~Logger() {
	PINFO("waiting for logger tasks to finish...\n");
	Logger::wait();
}

bool Logger::start(){
	Logger::current_lines = 0;
	PINFO("logfile path: '{}'\n", logpath);
	if(!fs::exists(logpath)){
		PINFO("creating log file '{}'\n", logpath);
		// create log file 
		if(!std::ofstream(logpath).is_open()){
			return false;
		}
	}
	else {
		std::ifstream file(logpath);
		str line;
		while (getline(file, line)) { ++current_lines; }
		file.close();
	}
	// TODO - change this 
	// set current lines

	// set max_lines
	max_lines = strtol(Config::get("logs","max_lines").data(),nullptr, 10);
	if(current_lines >= max_lines){
		return false;	
	}

	// set included_format_fields
	included_format_fields = Logger::FormatField::NONE;
	str token;
	sstream ss(Config::get("logs","included_format_fields").data());
	while(getline(ss, token, ':')){
		if(__str_ffield_map.find(token) != __str_ffield_map.end()){
			// it exists, add field
			included_format_fields = included_format_fields | __str_ffield_map[token];
		}
	}

	token.assign("");
	ss.clear();

	// set include_levels
	included_levels = Logger::Level::NONE;
	ss = std::move(sstream(Config::get("logs","included_levels").data()));
	while(getline(ss,token,':')){
		if(__str_level_map.find(token) != __str_level_map.end()){
			// it exists, add field
			included_levels = included_levels | __str_level_map[token];	
		}
	}

	return true;
} // Logger::start()

void Logger::wait() {
	for (std::future<void>& f : tasks) {
		f.wait();
	}
} // Logger::wait();

// log structure - [date] [level] [msg]
str Logger::format_msg(Level &level, str& msg) {
	sstream log;
	Logger::included_format_fields & Logger::FormatField::msg_time    && log << std::move(fmt::format("[{:%H:%M}] ", std::chrono::system_clock::now()));
	Logger::included_format_fields & Logger::FormatField::msg_type	  && log << std::move(fmt::format(__level_style_map[level],"[{}] ",__level_str_map[level]));
	Logger::included_format_fields & Logger::FormatField::msg_content && log << msg << EOL;
	return std::move(log.str());
} // Logger::format_msg();

