#pragma once
#include "../gpki.hpp"
#include "../logger/logger.hpp"

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
		const char* key_val_delim = "=";
		STYLE delim_styling = STYLE(fg(WHITE));
		ALLIGNMENT delim_allign = L_ALLIGN;
		int delim_width = 5;

		// Header
		STYLE header_styling = STYLE(fg(WHITE) | bg(BLACK) | EMPHASIS::bold);
		ALLIGNMENT header_allign = C_ALLIGN;
		int header_width = 10;

		// Key
		// std::pair<STYLE, ALLIGNMENT> key{ fg(WHITE) | EMPHASIS::bold, C_ALLIGN };
		STYLE key_styling = STYLE(fg(WHITE) | EMPHASIS::bold);
		ALLIGNMENT key_allign = C_ALLIGN;
		int key_width = 10;

		// Val
		STYLE val_styling = STYLE(fg(GREEN) | EMPHASIS::bold | EMPHASIS::italic);
		ALLIGNMENT val_allign = C_ALLIGN;
		int val_width = 10;
	}; // struct FormatInfo

	class Formatter
	{
	public:
		static FormatInfo gpkih_formatinfo();

		Formatter(FormatInfo format);
		void print_keyval(strview key, strview val) const;
		template <typename ...T> void print_headers(T&& ...header_names);

		const FormatInfo& get_format();
		void set_format(FormatInfo&& new_format);

	private:
		FormatInfo _format;
	}; // class Formatter

	// Print headers {:{width}} 
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
	
	inline FormatInfo Formatter::gpkih_formatinfo(){
		FormatInfo finfo;
		
		finfo.header_styling = STYLE(fg(CYAN) | bg(BLACK));
		finfo.header_width = 30; // looks more aesthetic with the same width as the key
		
		finfo.delim_width = 5;
		finfo.key_width = 30;   // max key length is 27 so 30 to make it fit
		
		finfo.val_width = 20;
		finfo.val_allign = L_ALLIGN;

		return finfo;
	}
} // namespace gpkih::experimental