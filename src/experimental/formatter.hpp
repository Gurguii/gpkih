#pragma once
#include "../logger/logger.hpp"
#include <fmt/color.h>

namespace gpkih::experimental
{
	enum class ALLIGNMENT : char
	{
		left = '<',
#define L_ALLIGN gpkih::experimental::ALLIGNMENT::left
		center = '^',
#define C_ALLIGN gpkih::experimental::ALLIGNMENT::center
		right = '>',
#define R_ALLIGN gpkih::experimental::ALLIGNMENT::right
	}; // enum ALLIGNMENT

	// Holds information related with 
	// formatting and styling
	struct FormatInfo {
		// Key val delimiter
		const char* key_val_delim;
		STYLE delim_styling;
		ALLIGNMENT delim_allign;
		int delim_width;

		// Header
		STYLE header_styling;
		ALLIGNMENT header_allign;
		int header_width;

		// Key
		STYLE key_styling;
		ALLIGNMENT key_allign;
		int key_width;

		// Val
		STYLE val_styling;
		ALLIGNMENT val_allign;
		int val_width;
	}; // struct FormatInfo

	class Formatter
	{
	public:
		FormatInfo static gpkih_formatinfo(); // FormatInfo struct with predefined values that fit gpkih.conf

		Formatter(FormatInfo format);
		void print_keyval(strview key, strview val, bool newline = true) const;
		template <typename ...T> void print_headers(T&& ...header_names);

		const FormatInfo& get_format();
		void set_format(FormatInfo&& new_format);

	private:
		FormatInfo _format;
	}; // class Formatter

	// Print headers {:{allign}{width}} 
	template <typename ...T> inline void Formatter::print_headers(T&& ...header_names)
	{
		sstream formatted{};
		for (int i = 0; i < sizeof ...(header_names); ++i) {
			formatted << "{:" << static_cast<char>(_format.header_allign) << std::move(fmt::format("{}", _format.header_width)) << "}";
		}
		// I don't like this other fmt::print("\n") but it's required so that header style doesn't 
		// get to the next line in a weird way (has an explanation 100% but rn idk)
		fmt::print(_format.header_styling,std::move(formatted.str()), std::forward<T>(header_names)...);
		fmt::print("\n");
	}

} // namespace gpkih::experimental

using namespace gpkih::experimental;
static inline constexpr FormatInfo gpkih_default_formatinfo(){
	return FormatInfo {
		.key_val_delim = "=",
		.delim_styling = fg(WHITE) | EMPHASIS::bold,
		.delim_allign = C_ALLIGN,
		.delim_width = 3,

		.header_styling = fg(fmt::terminal_color::cyan) | EMPHASIS::bold,
		.header_allign = C_ALLIGN,
		.header_width = 60,

		.key_styling = fg(WHITE) | EMPHASIS::bold,
		.key_allign = L_ALLIGN,
		.key_width = 30,

		.val_styling = fg(LGREEN) | EMPHASIS::bold,
		.val_allign = L_ALLIGN,
		.val_width = 30
	};
}