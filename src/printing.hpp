#pragma once

#include <fmt/chrono.h>
#include <fmt/color.h>
#include <iostream>
#include <string_view>

using COLOR = fmt::color;
using STYLE = fmt::text_style;
using EMPHASIS = fmt::emphasis;

static constexpr COLOR BLUE = COLOR::blue;
static constexpr COLOR RED = COLOR::red;
static constexpr COLOR GREEN = COLOR::green;
static constexpr COLOR YELLOW = COLOR::yellow;
static constexpr COLOR WHITE = COLOR::white;
static constexpr COLOR ORANGE = COLOR::orange;
static constexpr COLOR CYAN = COLOR::cyan;
static constexpr COLOR BLACK = COLOR::black;
static constexpr COLOR ALICIA = COLOR::alice_blue;
static constexpr COLOR LGREEN = COLOR::light_green;

static STYLE S_NONE = fg(BLACK);
static STYLE S_WARNING = fg(ORANGE) | EMPHASIS::bold | EMPHASIS::italic;
static STYLE S_INFO = fg(COLOR::pale_golden_rod) | EMPHASIS::italic;
static STYLE S_ERROR = fg(RED) | EMPHASIS::bold | EMPHASIS::italic;

// Styles for entity labels
static inline std::string S_ELABEL(std::string st) {
  return fmt::format(fg(COLOR::pale_golden_rod) | EMPHASIS::bold, st);
}

static inline std::string S_ELABEL_V(std::string st) {
  return fmt::format(fg(WHITE) | EMPHASIS::italic | EMPHASIS::bold, st);
}

// Styles for profile labels
static inline std::string S_PLABEL(std::string st) {
  return fmt::format(fg(COLOR::white) | EMPHASIS::bold, st);
}

static inline std::string S_PLABEL_V(std::string st) {
  return fmt::format(fg(LGREEN) | EMPHASIS::italic | EMPHASIS::bold, st);
}

/* NORMAL PRINTING */
template <typename... T> void PRINT(std::string_view msg, STYLE style) {
  fmt::print(style, msg);
};
static inline void PRINT(std::string_view msg, COLOR color) {
  fmt::print(fg(color), msg);
}

/* FORMATTED OUTPUT PRINTING */
template <typename... T>
void PRINTF(STYLE style, std::string fmt, T &&...args) {
  fmt::print(style, fmt, std::forward<T>(args)...);
};
template <typename... T> void PRINTF(std::string fmt, T &&...args) {
  fmt::print(fmt, std::forward<T>(args)...);
};

/* ERROR PRINTING */
template <typename... T> void PERROR(std::string fmt, T &&...args) {
  fmt::print(S_ERROR, " [error] " + fmt, std::forward<T>(args)...);
};

/* INFO PRINTING */
template <typename... T> void PINFO(std::string fmt, T &&...args) {
  fmt::print(S_INFO, " [info] " + fmt, std::forward<T>(args)...);
};

/* WARNING PRINTING*/
template <typename... T> void PWARN(std::string fmt, T &&...args) {
  fmt::print(S_WARNING, " [warning] " + fmt, std::forward<T>(args)...);
};

/* HINT PRINTING */
static inline void PHINT(std::string hint) {
  std::cout << fmt::format(fg(YELLOW), "💡 {}\n", hint);
}

/* PROMPT PRINTING */
static inline std::string PROMPT_icon() {
  return fmt::format(fg(GREEN), "➜ ");
};
static inline std::string PROMPT_body(std::string &body) {
  return fmt::format(fg(WHITE), "{}", body);
};
static inline std::string PROMPT_answers(std::string &ans) {
  return fmt::format(fg(WHITE) | EMPHASIS::italic | EMPHASIS::bold, "{}", ans);
};
static inline void PROMPT(std::string msg) {
  fmt::print(" {} {}", PROMPT_icon(), PROMPT_body(msg));
}
static inline void PROMPT(std::string msg, std::string ans) {
  fmt::print(" {} {} {}", PROMPT_icon(), PROMPT_body(msg), PROMPT_answers(ans));
}
static inline void PROGRAMSTARTING() {
  PINFO("Starting gpki - {:%d %h %Y @ %H:%M}\n",
        std::chrono::system_clock::now());
};
static inline void PROGRAMEXITING() {
  PINFO("Starting gpki - {:%d %h %Y @ %H:%M}\n",
        std::chrono::system_clock::now());
};
static inline void UNKNOWN_OPTION_MSG(std::string_view opt) {
  fmt::print(fg(COLOR::antique_white), " [parsing] unknown option '{}'\n", opt);
}
