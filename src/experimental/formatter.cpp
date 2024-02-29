#include "formatter.hpp"
#include <sstream>
using namespace gpkih::experimental;

Formatter::Formatter(FormatInfo finfo) :_format(finfo) {}

const FormatInfo& Formatter::get_format() {
	return _format;
} // Formatter::get_format()

void Formatter::set_format(FormatInfo&& new_format) {
	// idk if this is required but just in case
	_format = std::move(new_format);
} // Formatter::set_format()

void Formatter::print_keyval (strview key, strview val, bool newline) const {
	sstream formatted{};

	// Add placeholders with desired allignment and width for each component - [key|delim|val]
	// this will essentially create stuff like:
	// {:^10}{:>10}{:<10} which will be the placeholders for  [key - delim - value]
	str styled_key_placeholder = std::move(fmt::format("{}{}{}{}","{:",static_cast<char>(_format.key_allign), _format.key_width, "}")); // "{:^10}"
	str styled_delim_placeholder = std::move(fmt::format("{}{}{}{}", "{:", static_cast<char>(_format.delim_allign), _format.delim_width, "}"));
	str styled_val_placeholder = std::move(fmt::format("{}{}{}{}", "{:", static_cast<char>(_format.val_allign), _format.val_width, "}"));

	fmt::print(_format.key_styling, styled_key_placeholder, key);
	fmt::print(_format.delim_styling, styled_delim_placeholder, _format.key_val_delim);
	fmt::print(_format.val_styling, styled_val_placeholder, val);

	if(newline){
		fmt::print("\n");
	}
}

