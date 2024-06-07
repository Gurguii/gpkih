#include "printing.hpp"
#include <iostream>

bool ENABLE_DEBUG_MESSAGES = false;
int DEBUG_LEVEL = 1;
bool ENABLE_PRINTING = true;

/* Normal printing */
void PRINT(std::string_view msg, COLOR color) {
	fmt::print(fg(color), msg);
}
void PRINT(std::string_view msg, STYLE style) {
	fmt::print(style, msg);
}

/* Used to style PROMPT components: icon - body - answers */
static inline std::string PROMPT_icon(COLOR icon_color) {
	return fmt::format(fg(icon_color), "-> ");
};
static inline std::string PROMPT_body(std::string_view body) {
	return fmt::format(fg(WHITE), "{}", body);
};
static inline std::string PROMPT_answers(std::string_view ans) {
	return fmt::format(fg(WHITE) | EMPHASIS::italic | EMPHASIS::bold, "{}", ans);
};

std::string PROMPT(std::string_view msg, bool lower_input, COLOR icon_color) {
	std::string input = "";
	fmt::print("{} {}: ", PROMPT_icon(icon_color), PROMPT_body(msg));
	std::getline(std::cin, input);
	if(lower_input){
		for(char &c : input){
			c = tolower(c);
		}
	}
	return std::move(input);
};

std::string PROMPT(std::string_view msg, std::string_view ans, bool lower_input, COLOR icon_color){
	std::string input = "";
	fmt::print("{} {} {}: ", PROMPT_icon(icon_color), PROMPT_body(msg), PROMPT_answers(ans));
	std::getline(std::cin, input);
	if(lower_input){
		for(char &c : input){
			c = tolower(c);
		}
	}		
	return std::move(input);
}

void PROGRAMSTARTING() {
	// e.g 06 Mar 2024 @ 19:13
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

std::unordered_map<std::string, COLOR> map_str_color(){
	return {
		{"BLUE", BLUE},
		{"RED", RED},
		{"GREEN", GREEN},
		{"YELLOW",  YELLOW},
		{"WHITE", WHITE},
		{"ORANGE", ORANGE},
		{"CYAN", CYAN},
		{"BLACK", BLACK},
		{"ALICIA", ALICIA},
		{"LGREEN", LGREEN},
		{"PGR", PALE_GOLDEN_ROD},
		{"LPINK", LPINK},
		{"PINK", PINK}
	};
}