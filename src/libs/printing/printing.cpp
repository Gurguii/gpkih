#include "printing.hpp"
#include <iostream>

void UNKNOWN_OPTION_MESSAGE(std::string_view opt) {
	fmt::print(fg(COLOR::antique_white), " [parsing] unknown option '{}'\n", opt);
};

bool ENABLE_DEBUG_MESSAGES = false;
bool ENABLE_PRINTING = true;
int MAX_DEBUG_LEVEL = 1;

/* Used to style prompt components: icon - body - answers */
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

const std::unordered_map<std::string, COLOR>& map_str_color(){
	static std::unordered_map<std::string, COLOR>_map{
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
	return _map;
}

void printDebugMsg(int debugLevel, const char *fp, int ln, std::string_view msg){
	if(debugLevel > MAX_DEBUG_LEVEL){
		return;
	}
	static int offset = 0;
	if(offset == 0){
		// set offset
		size_t len = strlen(fp);
		for(int i = 0; i < len; ++i){
			if(fp[i] == '/' && fp[i+1] == 's' && fp[i+2] == 'r' && fp[i+3] == 'c'){
				offset = i+5;
			}
		}
	}
	if(ENABLE_DEBUG_MESSAGES){
		printf("%s (%i) - %s\n",fp+offset, ln, msg.data());
	}
}