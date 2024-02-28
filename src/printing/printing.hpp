#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <string_view>

using COLOR = fmt::color;
using STYLE = fmt::text_style;
using EMPHASIS = fmt::emphasis;

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

/* Defined styles for printing */
extern STYLE S_NONE;
extern STYLE S_WARNING;
extern STYLE S_INFO;
extern STYLE S_ERROR;
extern STYLE S_SUCCESS;

/* Styling used in action 'list' */
extern STYLE ENTITY_LABEL_KEY_STYLE;
extern STYLE ENTITY_LABEL_VAL_STYLE;

extern STYLE PROFILE_LABEL_KEY_STYLE;
extern STYLE PROFILE_LABEL_VAL_STYLE;

/* Returns styled string */
extern std::string S_ELABEL(std::string& st);
extern std::string S_ELABEL_V(std::string& st);

extern std::string S_PLABEL(std::string& st);
extern std::string S_PLABEL_V(std::string& st);

/* Normal printing */
extern void PRINT(std::string_view msg, COLOR color);
extern void PRINT(std::string_view msg, STYLE style);

/* success */
template <typename ...T> extern void PSUCCESS(std::string_view fmt, T&& ...args);
/* error */
template <typename ...T> extern void PERROR(std::string_view fmt, T&& ...args);
/* info */
template <typename ...T> extern void PINFO(std::string_view fmt, T&& ...args);
/* warning */
template <typename ...T> extern void PWARN(std::string_view fmt, T&& ...args);
/* hint */
template <typename ...T> extern void PHINT(std::string_view fmt, T&& ...args);
/* prompt */
extern std::string PROMPT_icon();
extern std::string PROMPT_body();
extern std::string PROMPT_answers();
extern void PROMPT; // Leave this one here, the PROMPT_... functions can be staticly declared in printing.cpp

static inline std::string PROMPT_icon(COLOR icon_color = GREEN) {
    return fmt::format(fg(icon_color), "➜ ");
};
static inline std::string PROMPT_body(std::string& body) {
    return fmt::format(fg(WHITE), "{}", body)
};
static inline std::string PROMPT_answers(std::string& ans) {
    return fmt::format(fg(WHITE) | EMPHASIS::italic | EMPHASIS::bold, "{}", ans);
};
static inline void PROMPT(std::string msg, COLOR icon_color = GREEN) {
    fmt::print(" {} {}", PROMPT_icon(icon_color), PROMPT_body(msg));
}
static inline void PROMPT(std::string msg, std::string ans,
    COLOR icon_color = GREEN) {
    fmt::print(" {} {} {}", PROMPT_icon(icon_color), PROMPT_body(msg),
        PROMPT_answers(ans));

// Message to print when starting
extern void PROGRAMSTARTING();
// Message to print when exiting
extern void PROGRAMEXITING();

// Message to print when an unknown option is found when parsing
extern void UNKNOWN_OPTION_MSG(std::string_view opt);