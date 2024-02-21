#pragma once

#include <fmt/format.h>
#include "../printing.hpp"

#include <string_view>
#include <queue>
#include <future>

namespace gpkih
{
class Logger
{
private:
	std::queue<std::string> message_queue;
	std::string file;
public:
	enum class Level{
		INFO,
		WARNING,
		ERROR
	};
	Logger(std::string st);
	template<typename ...T>void add(Level level, std::string_view fmt, T&& ...args);
};
} // namespace gpkih