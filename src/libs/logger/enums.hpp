#pragma once
#include <cstdint>
#include <cstddef>

enum class LoggerRCode : int
{
	ok  = 0,
	fail = -1,
	level_not_included = 1,
	max_size_reached   = 2,
	path_not_absolute  = 3,
	cant_create_file   = 4,
};
constexpr static int L_OK = static_cast<int>(LoggerRCode::ok);
constexpr static int L_FAIL = static_cast<int>(LoggerRCode::fail);
constexpr static int L_LEVEL_NOT_INCLUDED = static_cast<int>(LoggerRCode::level_not_included);
constexpr static int L_MAX_LOG_REACHED = static_cast<int>(LoggerRCode::max_size_reached);
constexpr static int L_PATH_NOT_ABSOLUTE = static_cast<int>(LoggerRCode::path_not_absolute);
constexpr static int L_CANT_CREATE_FILE = static_cast<int>(LoggerRCode::cant_create_file);

enum class logLevel : uint8_t 
{
	none    = 0x00,
	info    = 0x01,
	warning = 0x02,
	error   = 0x04,
	all     = 0x07
};
constexpr static logLevel LL_NONE = logLevel::none;
constexpr static logLevel LL_INFO = logLevel::info;
constexpr static logLevel LL_WARN = logLevel::warning;
constexpr static logLevel LL_ERROR = logLevel::error;

enum class logMsgField : uint16_t
{
	none    = 0x00,
	type    = 0x01,
	content = 0x02,
	time    = 0x04,
	all     = 0x07
};
constexpr static logMsgField LF_NONE = logMsgField::none;
constexpr static logMsgField LF_TYPE = logMsgField::type;
constexpr static logMsgField LF_CONTENT = logMsgField::content;
constexpr static logMsgField LF_TIME = logMsgField::time;
constexpr static logMsgField LF_ALL = logMsgField::all;

struct logMetadata{
	size_t infoCount   = 0;
	size_t warnCount   = 0;
	size_t errCount    = 0;
	size_t entries     = 0;
	size_t currentSize = 0;
};

static inline logMsgField operator|(logMsgField lo, logMsgField ro) {
	return static_cast<logMsgField>(static_cast<uint16_t>(lo) | static_cast<uint16_t>(ro));
} // logMsgField operator |
static inline bool operator&(logMsgField lo, logMsgField ro) {
	return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
} // logMsgField operator &
static inline logLevel operator|(logLevel lo, logLevel ro) {
	return static_cast<logLevel>(static_cast<uint16_t>(lo) | static_cast<uint16_t>(ro));
} // logLevel operator |
static inline bool operator&(logLevel lo, logLevel ro) {
	return static_cast<bool>(static_cast<uint16_t>(lo) & static_cast<uint16_t>(ro));
} // logLevel operator &