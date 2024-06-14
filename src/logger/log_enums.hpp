#pragma once
#include <cstdint>
#include <filesystem>

enum class logLevel
{
	none = 0,
	info = 2,
	warning = 4,
	error = 8,
	all = 15
};
constexpr static logLevel L_NONE = logLevel::none;
constexpr static logLevel L_INFO = logLevel::info;
constexpr static logLevel L_WARN = logLevel::warning;
constexpr static logLevel L_ERROR = logLevel::error;

enum class logMsgField : uint16_t
{
	none = 0,
	type = 2,
	content = 4,
	time = 8,
	all = 15
};

struct logMetadata{
	size_t infoCount;
	size_t warnCount;
	size_t errCount;
	size_t entries;
	size_t currentSize;
	std::filesystem::path filePath;
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