#include "logger.hpp"

int main()
{
	gurgui::logging::Logger::setBaseDir(".");
	gurgui::logging::Logger logger("test.log", 400000, logMsgField::all, logLevel::all);
	logger.addLog(LL_INFO, "testing Logger class");
	return 0;
}