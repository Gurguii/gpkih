#include "formatter.hpp"

using namespace gpkih::experimental;

Formatter::Formatter(FormatInfo finfo) :_format(finfo) {}


Formatter::FormatInfo& Formatter::get_format() {
	return _format;
} // Formatter::get_format()

void Formatter::set_format(FormatInfo&& new_format) {
	// idk if this is required but just in case
	_format = std::move(new_format);
} // Formatter::set_format()