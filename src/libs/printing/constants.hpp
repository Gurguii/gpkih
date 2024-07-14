#pragma once
#include <fmt/format.h>
#include <fmt/color.h>

using COLOR    = fmt::color;
using STYLE    = fmt::text_style;
using EMPHASIS = fmt::emphasis;
using TCOLOR   = fmt::terminal_color;

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