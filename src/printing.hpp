#pragma once
#include <fmt/base.h>
#include <fmt/chrono.h>
#include <iostream>
#include <fmt/color.h>
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

STYLE S_NONE = fg(BLACK); 
STYLE  S_WARNING = fg(ORANGE) | EMPHASIS::underline;
STYLE S_INFO = fg(COLOR::pale_golden_rod) | EMPHASIS::italic;
STYLE S_ERROR = fg(RED) | bg(BLACK) | EMPHASIS::underline;

// Styles for entity labels
static inline std::string S_ELABEL(std::string st) {
    return fmt::format(fg(COLOR::pale_golden_rod) | EMPHASIS::bold, st);
}

static inline std::string S_ELABEL_V(std::string st) {
    return fmt::format(fg(WHITE) | EMPHASIS::italic | EMPHASIS::bold, st);
}

// Styles for profile labels
static inline std::string S_PLABEL(std::string st) {
    return fmt::format(fg(COLOR::white) |  EMPHASIS::bold, st);
}

static inline std::string S_PLABEL_V(std::string st) {
    return fmt::format(fg(GREEN) | EMPHASIS::italic | EMPHASIS::bold, st);
}

/* NORMAL PRINTING */
template <typename ...T> void PRINT(std::string_view msg, STYLE style){
    fmt::print(style,msg);
};
static inline void PRINT(std::string_view msg, COLOR color){
    fmt::print(fg(color),msg);
}

/* FORMATTED OUTPUT PRINTING */
template <typename ...T> void PRINTF(STYLE style, std::string fmt, T&&... args){
    fmt::print(style,fmt,std::forward<T>(args)...);
};
template <typename ...T> void PRINTF(std::string fmt, T&&... args){
    fmt::print(fmt,std::forward<T>(args)...);
};

/* ERROR PRINTING */
template <typename ...T> void PERROR(std::string fmt, T&&... args){
    fmt::print(S_ERROR,fmt,std::forward<T>(args)...);
};
static inline void PERROR(std::string msg){
    fmt::print(S_ERROR," [error] "+ msg);
};

/* INFO PRINTING */
template <typename ...T> void PINFO(std::string fmt, T&&... args){
    fmt::print(S_INFO," [info] " + fmt,std::forward<T>(args)...);
};
static inline void PINFO(std::string msg){
    fmt::print(S_INFO," [info] "+ msg);
};

/* WARNING PRINTING*/
template <typename ...T> void PWARN(std::string fmt, T&&... args){
    fmt::print(S_WARNING, fmt,std::forward<T>(args)...);
};
static inline void PWARN(std::string msg){
    fmt::print(S_WARNING," [warning] " + msg );
};

static inline void PROGRAMSTARTING(){
    PINFO("Starting gpki - {:%d %h %Y @ %H:%M}\n", std::chrono::system_clock::now());
};

static inline void UNKNOWN_OPTION_MSG(std::string_view opt) {
  fmt::print(fg(COLOR::antique_white)," unknown option '{}'",opt);
}