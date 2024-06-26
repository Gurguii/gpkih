#pragma once

#include "../libs/printing/printing.hpp"
#include <sstream>

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
			Formatter(FormatInfo format);
	
			void print_keyval(std::string_view key, std::string_view val, bool newline = true) const;
			template <typename T> void print_keyval (std::string_view key, T val, bool newline = true) const;
	
			template <typename ...T> void print_headers(T&& ...header_names);
			template <typename ...T> void print_entry(T&& ...entry_vals);

			const FormatInfo& get_format();
			void set_format(FormatInfo&& new_format);
	
		private:
			FormatInfo _format;
	}; // class Formatter

	// Print headers {:{allign}{width}} 
	template <typename ...T> inline void Formatter::print_headers(T&& ...header_names)
	{
		std::stringstream formatted{};
		for (int i = 0; i < sizeof ...(header_names); ++i) {
			formatted << "{:" << static_cast<char>(_format.header_allign) << std::move(fmt::format("{}", _format.header_width)) << "}";
		}
		// I don't like this other fmt::print("\n") but it's required so that header style doesn't 
		// get to the next line in a weird way (has an explanation 100% but rn idk)
		fmt::print(_format.header_styling,std::move(formatted.str()), std::forward<T>(header_names)...);
		fmt::print("\n");
	}

	template <typename T> void Formatter::print_keyval (std::string_view key, T val, bool newline) const {
		// Add placeholders with desired allignment and width for each component - [key|delim|val]
		// this will essentially create stuff like:
		// {:^10}{:>10}{:<10} which will be the placeholders for  [key - delim - value]
		std::string styled_key_placeholder = std::move(fmt::format(_format.key_styling,"{}{}{}{}","{:",static_cast<char>(_format.key_allign), _format.key_width, "}")); // "{:^10}"
		std::string styled_delim_placeholder = std::move(fmt::format(_format.delim_styling, "{}{}{}{}", "{:", static_cast<char>(_format.delim_allign), _format.delim_width, "}"));
		std::string styled_val_placeholder = std::move(fmt::format(_format.val_styling,"{}{}{}{}", "{:", static_cast<char>(_format.val_allign), _format.val_width, "}"));
	
		fmt::print(
			fmt::format("{}{}{}", 
				styled_key_placeholder, styled_delim_placeholder, styled_val_placeholder
			), key, _format.key_val_delim, val
		);
	
		if(newline){
			fmt::print("\n");
		}
	}	

	template <typename ...T> void Formatter::print_entry(T&& ...entry_vals){
		std::stringstream formatted{};
		for(auto i = 0; i < sizeof...(entry_vals); ++i){
			formatted << "{:^10}";
		}
		fmt::print(formatted.str(), std::forward<T>(entry_vals)...);
		fmt::print("\n");
    }

} // namespace gpkih::experimental

using namespace gpkih::experimental;
static inline FormatInfo gpkih_default_formatinfo(){
	
	FormatInfo finfo;
	finfo.key_val_delim = "=";
	finfo.delim_styling = fg(WHITE) | EMPHASIS::bold;
	finfo.delim_allign = C_ALLIGN;
	finfo.delim_width = 3;
	finfo.header_styling = fg(fmt::terminal_color::cyan) | EMPHASIS::bold;
	finfo.header_allign = C_ALLIGN;
	finfo.header_width = 60;
	finfo.key_styling = fg(WHITE) | EMPHASIS::bold;
	finfo.key_allign = L_ALLIGN;
	finfo.key_width = 30;
	finfo.val_styling = fg(LGREEN) | EMPHASIS::bold;
	finfo.val_allign = L_ALLIGN;
	finfo.val_width = 30;

	return finfo;
}