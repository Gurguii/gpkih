#pragma once
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/format.h>

#include <string_view>
#include <unordered_map>

extern bool ENABLE_DEBUG_MESSAGES;
extern int DEBUG_LEVEL;
extern bool ENABLE_PRINTING;

using COLOR = fmt::color;
using STYLE = fmt::text_style;
using EMPHASIS = fmt::emphasis;
using TCOLOR = fmt::terminal_color;

constexpr TCOLOR T_BLUE = TCOLOR::blue;
constexpr TCOLOR TB_BLUE = TCOLOR::bright_blue;
constexpr TCOLOR T_RED = TCOLOR::red;
constexpr TCOLOR TB_RED = TCOLOR::bright_red;
constexpr TCOLOR T_GREEN = TCOLOR::green;
constexpr TCOLOR TB_GREEN = TCOLOR::bright_green;
constexpr TCOLOR T_YELLOW = TCOLOR::yellow;
constexpr TCOLOR TB_YELLOW = TCOLOR::bright_yellow;
constexpr TCOLOR T_WHITE = TCOLOR::white;

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

constexpr STYLE S_NONE = fg(T_WHITE);
constexpr STYLE S_INFO = fg(T_YELLOW) | EMPHASIS::italic;
constexpr STYLE S_WARNING = fg(TB_YELLOW) | EMPHASIS::bold | EMPHASIS::italic;
constexpr STYLE S_ERROR = fg(T_RED) | EMPHASIS::bold | EMPHASIS::italic;
constexpr STYLE S_SUCCESS = fg(T_GREEN) | EMPHASIS::bold;
constexpr STYLE S_HINT = fg(T_YELLOW) | EMPHASIS::italic;

 
#ifdef GPKIH_ENABLE_DEBUGGING
inline static constexpr bool DEBUG_COMPILED = true;
#else
inline static constexpr bool DEBUG_COMPILED = false;
#endif

/* Normal printing */
extern void PRINT(std::string_view msg, COLOR color);
extern void PRINT(std::string_view msg, STYLE style = S_NONE);

/* success */
template <typename ...T> static inline void PSUCCESS(std::string_view fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print("{} {}",fmt::format(S_SUCCESS, "[success]"), fmt::format(fmt, std::forward<T>(args)...));};
};

/* error */
template <typename ...T> static inline void PERROR(std::string_view fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_ERROR, "[error] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* info */
template <typename ...T> static inline void PINFO(std::string_view fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_INFO, "[info] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* warning */
template <typename ...T> static inline void PWARN(std::string_view fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_WARNING, "[warn] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* hint */
template <typename ...T> static inline void PHINT(std::string_view fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_HINT, "[hint] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* debug */
template <typename ...T> static void PDEBUG(int dlevel, std::string_view fmt, T&& ...args) {
#ifdef GPKIH_ENABLE_DEBUGGING
	if(ENABLE_DEBUG_MESSAGES && (dlevel <= DEBUG_LEVEL)){
		switch(dlevel){
			case 1:
				fmt::print("-- {}\n", fmt::format(fmt, std::forward<T>(args)...));
				break;
			;;
			case 2:
				fmt::print("  {}\n", fmt::format(fmt, std::forward<T>(args)...));
				break;
			;;
			case 3:
				fmt::print("    {}\n", fmt::format(fmt, std::forward<T>(args)...));
				break;
			;;
			default:
				break;
			;;
		}		
	}
#else
	return;
#endif
}

/* prompt */
extern std::string PROMPT(std::string_view msg, std::string_view ans, bool lower_input = false, COLOR icon_color = LGREEN);
extern std::string PROMPT(std::string_view msg, bool lower_input = false, COLOR icon_color = LGREEN);

// Message to print when an unknown option is found when parsing
extern void UNKNOWN_OPTION_MESSAGE(std::string_view opt);

extern std::unordered_map<std::string, COLOR> map_str_color();