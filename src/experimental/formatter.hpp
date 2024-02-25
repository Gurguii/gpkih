#pragma once
#include "../gpki.hpp"

namespace gpkih::experimental
{
	class Formatter
	{
	public:
		enum class ALLIGNMENT : char
		{
			left = '<',
		#define L_ALLIGN ALLIGNMENT::left
			center = '^',
		#define C_ALLIGN ALLIGNMENT::center
			right = '>',
		#define R_ALLIGN ALLIGNMENT::right
		};

		struct FormatInfo {
			int width = 10;
			ALLIGNMENT allignment = C_ALLIGN;
		};

		Formatter(FormatInfo format);

		template <typename ...T> void printh(T&& ...args) {
			std::stringstream format_string{};
			for (int i = 0; i < sizeof...(args); ++i) {
				format_string << "{:" << static_cast<char>(info.allignment) << std::move(fmt::format("{}", info.width)) << "}";
			}
			fmt::print(format_string.str(), std::forward<T>(args)...);
		}
		template <typename ...T> void printv(T&& ...args);
	private:
		FormatInfo _format;
	};

} // namespace gpkih::experimental

template <typename ...T> inline void gpkih::experimental::Formatter::printv(T&& ...args)
{
	std::stringstream format_string{};
	for (int i = 0; i < sizeof...(args); ++i) {
		format_string << "{:" << static_cast<char>(info.allignment) << std::move(fmt::format("{}", info.width)) << "}\n";
	}
	fmt::print(format_string.str(), std::forward<T>(args)...);
}