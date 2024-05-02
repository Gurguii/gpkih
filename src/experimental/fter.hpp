#pragma once

#include <sstream>
#include <string>
#include <string_view>

#include "../printing/printing.hpp"
#include "../structs.hpp"

using namespace gpkih;

namespace Fter
{
	extern void pentry(Entity &entity, STYLE st = S_NONE);
	extern void pentry(Profile &profile, STYLE st = S_NONE);

	template <typename ...T> void pheaders(int hwidth, T&& ...args);
	template <typename ...T> void pheaders(int hwidth, T&& ...args);
	template <typename T> void pkeyval(std::string_view key, T val);
};