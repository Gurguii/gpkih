#include "printing.hpp"
#include <string_view>

/* Defined styles for printing */
STYLE S_NONE = fg(BLACK);
STYLE S_WARNING = fg(ORANGE) | EMPHASIS::bold | EMPHASIS::italic;
STYLE S_INFO = fg(PALE_GOLDEN_ROD) | EMPHASIS::italic;
STYLE S_ERROR = fg(RED) | EMPHASIS::bold | EMPHASIS::italic;
STYLE S_SUCCESS = fg(LGREEN) | EMPHASIS::bold;

/* Styling used in action 'list' */
STYLE ENTITY_LABEL_KEY_STYLE = fg(PALE_GOLDEN_ROD) | EMPHASIS::bold;
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

/* Used to style PROMPT components: icon - body - answers */
static inline std::string PROMPT_icon(COLOR icon_color) {
	return fmt::format(fg(icon_color), "➜ ");
};
static inline std::string PROMPT_body(std::string_view body) {
	return fmt::format(fg(WHITE), "{}", body);
};
static inline std::string PROMPT_answers(std::string_view ans) {
	return fmt::format(fg(WHITE) | EMPHASIS::italic | EMPHASIS::bold, "{}", ans);
};

void PROMPT(std::string_view msg, std::string_view ans, COLOR icon_color) {
	fmt::print("{} {} {}: ", PROMPT_icon(icon_color), PROMPT_body(msg), PROMPT_answers(ans));
};

void PROGRAMSTARTING() {
	PINFO("Starting gpkih - {:%d %h %Y @ %H:%M}\n",
		std::chrono::system_clock::now());
};

void PROGRAMEXITING() {
	PINFO("Exiting gpkih - {:%d %h %Y @ %H:%M}\n",
		std::chrono::system_clock::now());
};

void UNKNOWN_OPTION_MESSAGE(std::string_view opt) {
	fmt::print(fg(COLOR::antique_white), " [parsing] unknown option '{}'\n", opt);
};