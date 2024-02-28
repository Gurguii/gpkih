#include "printing.hpp"


/* Defined styles for printing */
STYLE S_NONE = fg(BLACK);
STYLE S_WARNING = fg(ORANGE) | EMPHASIS::bold | EMPHASIS::italic;
STYLE S_INFO = fg(PALE_GOLDEN_ROD) | EMPHASIS::italic;
STYLE S_ERROR = fg(RED) | EMPHASIS::bold | EMPHASIS::italic;
STYLE S_SUCCESS = fg(LGREEN) | EMPHASIS::bold;

/* Styling used in action 'list' */
STYLE ENTITY_LABEL_KEY_STYLE = fg(PALE_GOLDEN_ROD) | EMPHASIS::bold);
STYLE ENTITY_LABEL_VAL_STYLE = fg(WHITE) | EMPHASIS::italic | EMPHASIS::bold;

STYLE PROFILE_LABEL_KEY_STYLE = fg(WHITE) | EMPHASIS::bold;
STYLE PROFILE_LABEL_VAL_STYLE = fg(LGREEN) | EMPHASIS::italic | EMPHASIS::bold;

std::string S_ELABEL(std::string& st){
	return std::move(fmt::format(ENTITY_LABEL_KEY_STYLE, st));
}
std::string S_ELABEL_V(std::string& st){
	return std::move(fmt::format(ENTITY_LABEL_VAL_STYLE, st));
}

std::string S_PLABEL(std::string& st) {
	return std::move(fmt::format(PROFILE_LABEL_KEY_STYLE, st));
}
std::string S_PLABEL_V(std::string& st) {
	return std::move(fmt::format(PROFILE_LABEL_VAL_STYLE, st));
}

/* Normal printing */
void PRINT(std::string_view msg, COLOR color) {
	fmt::print(fg(color), msg);
}
void PRINT(std::string_view msg, STYLE style) {
	fmt::print(style, msg);
}

/* success */
template <typename ...T> void PSUCCESS(std::_String_val fmt, T&& ...args) {
	fmt::print(S_SUCCESS, fmt, std::forward<T>(args)...);
}

/* */