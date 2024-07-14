#pragma once
#include <string_view>
#include <unordered_map>
#include "constants.hpp"

extern bool ENABLE_DEBUG_MESSAGES; // default false
extern bool ENABLE_PRINTING; // default true
extern int MAX_DEBUG_LEVEL;  // default 1

/* prompt */
extern std::string PROMPT(std::string_view msg, std::string_view ans, bool lower_input = false, COLOR icon_color = LGREEN);
extern std::string PROMPT(std::string_view msg, bool lower_input = false, COLOR icon_color = LGREEN);

extern const std::unordered_map<std::string, COLOR>& map_str_color();

/* success */
template <typename ...T> static inline void PSUCCESS(const char *fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print("{} {}", fmt::format(S_SUCCESS, "[success]"), fmt::format(fmt, std::forward<T>(args)...));};
};

/* error */
template <typename ...T> static inline void PERROR(const char *fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_ERROR, "[error] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* info */
template <typename ...T> static inline void PINFO(const char *fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_INFO, "[info] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* warning */
template <typename ...T> static inline void PWARN(const char *fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_WARNING, "[warn] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* hint */
template <typename ...T> static inline void PHINT(const char *fmt, T&& ...args){
	if(ENABLE_PRINTING){fmt::print(S_HINT, "[hint] {}", fmt::format(fmt, std::forward<T>(args)...));};
};

/* debug */
#ifdef GPRINTING_ENABLE_DEBUGGING
extern void printDebugMsg(int debugLevel, const char *filePath, int lineNo, std::string_view msg);
// TODO find a usage for debug level
#define DEBUG(lvl, msg) printDebugMsg(lvl, __FILE__,__LINE__,msg);
#define DEBUGF(lvl, msg, ...) printDebugMsg(lvl, __FILE__,__LINE__,fmt::format(msg,__VA_ARGS__))
#else
#define DEBUG(lvl, msg)
#define DEBUGF(lvl, msg, ...)
#endif

extern void UNKNOWN_OPTION_MESSAGE(std::string_view opt);