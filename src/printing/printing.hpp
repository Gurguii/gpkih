#pragma once
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>

#include <string_view>
#include <unordered_map>

#include <iostream>

extern bool ENABLE_DEBUG_MESSAGES;
extern int DEBUG_LEVEL;

using COLOR = fmt::color;
using STYLE = fmt::text_style;
using EMPHASIS = fmt::emphasis;
using TCOLOR = fmt::terminal_color;

constexpr TCOLOR TBLUE = TCOLOR::blue;
constexpr TCOLOR TRED = TCOLOR::red;
constexpr TCOLOR TGREEN = TCOLOR::green;
constexpr TCOLOR TYELLOW = TCOLOR::yellow;
constexpr TCOLOR TWHITE = TCOLOR::white;

constexpr COLOR BLUE = COLOR::blue;
constexpr COLOR RED = COLOR::red;
constexpr COLOR GREEN = COLOR::green;
constexpr COLOR YELLOW = COLOR::yellow;
constexpr COLOR WHITE = COLOR::white;
constexpr COLOR ORANGE = COLOR::orange;
constexpr COLOR CYAN = COLOR::cyan;
constexpr COLOR BLACK = COLOR::black;
constexpr COLOR ALICIA = COLOR::alice_blue;
constexpr COLOR LGREEN = COLOR::light_green;
constexpr COLOR PALE_GOLDEN_ROD = COLOR::pale_golden_rod;
constexpr COLOR LPINK = COLOR::light_pink;
constexpr COLOR PINK = COLOR::pink;

/* Defined styles for printing */
extern STYLE S_NONE;
extern STYLE S_WARNING;
extern STYLE S_INFO;
extern STYLE S_ERROR;
extern STYLE S_SUCCESS;
extern STYLE S_HINT;

/* Styling used in action 'list' */
extern STYLE ENTITY_LABEL_KEY_STYLE;
extern STYLE ENTITY_LABEL_VAL_STYLE;

extern STYLE PROFILE_LABEL_KEY_STYLE;
extern STYLE PROFILE_LABEL_VAL_STYLE;

/* Returns styled string */
extern std::string S_ELABEL(std::string_view st);
extern std::string S_ELABEL_V(std::string_view st);

extern std::string S_PLABEL(std::string_view st);
extern std::string S_PLABEL_V(std::string_view st);

/* Normal printing */
extern void PRINT(std::string_view msg, COLOR color);
extern void PRINT(std::string_view msg, STYLE style = S_NONE);

template <typename T> void foo(){}
extern template void foo<int>();

/* success */
template <typename ...T> static inline void PSUCCESS(std::string_view fmt, T&& ...args){
	fmt::print("{} {}", fmt::format(S_SUCCESS, "[success]"), fmt::format(fmt, std::forward<T>(args)...));
};

/* error */
template <typename ...T> static inline void PERROR(std::string_view fmt, T&& ...args){
	fmt::print(S_ERROR, "[error] {}", fmt::format(fmt, std::forward<T>(args)...));
};

/* info */
template <typename ...T> static inline void PINFO(std::string_view fmt, T&& ...args){
	fmt::print(S_INFO, "[info] {}", fmt::format(fmt, std::forward<T>(args)...));
};

/* warning */
template <typename ...T> static inline void PWARN(std::string_view fmt, T&& ...args){
	fmt::print(S_WARNING, "[warn] {}", fmt::format(fmt, std::forward<T>(args)...));
};

/* hint */
template <typename ...T> static inline void PHINT(std::string_view fmt, T&& ...args){
	fmt::print(S_HINT, "[hint] {}", fmt::format(fmt, std::forward<T>(args)...));
};

/* debug */
template <typename ...T> static inline void PDEBUG(int dlevel, std::string_view fmt, T&& ...args) {
	if(ENABLE_DEBUG_MESSAGES && (dlevel <= DEBUG_LEVEL)){
		fmt::print("-- {}\n", fmt::format(fmt, std::forward<T>(args)...));
	}
}

/* prompt */
extern std::string PROMPT(std::string_view msg, std::string_view ans, bool lower_input = false, COLOR icon_color = LGREEN);
extern std::string PROMPT(std::string_view msg, bool lower_input = false, COLOR icon_color = LGREEN);

// Message to print when starting
extern void PROGRAMSTARTING();

// Message to print when exiting
extern void PROGRAMEXITING();

// Message to print when an unknown option is found when parsing
extern void UNKNOWN_OPTION_MESSAGE(std::string_view opt);

extern std::unordered_map<std::string, COLOR> map_str_color();