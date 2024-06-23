#pragma once
#include <string>
#include <unordered_map>
#include <cstdint>

static inline constexpr char sectionOpenDelim = '[';
static inline constexpr char sectionCloseDelim = ']';
static inline constexpr const char *emptyChars = "[] ";
static inline constexpr const char *gpkihSections = "logs behaviour cli formatting";
static inline constexpr const char *skipChars = "#\n ";

using ConfigMap = std::unordered_map<std::string, std::unordered_map<std::string, std::string>>;

enum class CONFIG_FILE
{
  none  = 0x00,
  gpkih = 0x01,
  vpn   = 0x02,
  pki   = 0x04,
  all   = 0x07,
};  
constexpr CONFIG_FILE CFILE_NONE  = CONFIG_FILE::none;
constexpr CONFIG_FILE CFILE_ALL   = CONFIG_FILE::all;
constexpr CONFIG_FILE CFILE_VPN   = CONFIG_FILE::vpn;
constexpr CONFIG_FILE CFILE_PKI   = CONFIG_FILE::pki;
constexpr CONFIG_FILE CFILE_GPKIH = CONFIG_FILE::gpkih;

static inline CONFIG_FILE operator|(CONFIG_FILE lo, CONFIG_FILE ro){
  return static_cast<CONFIG_FILE>(static_cast<uint8_t>(lo) |
                                  static_cast<uint8_t>(ro));
}

static inline bool operator&(CONFIG_FILE lo, CONFIG_FILE ro){
  return static_cast<bool>(static_cast<uint8_t>(lo) & static_cast<uint8_t>(ro));
}