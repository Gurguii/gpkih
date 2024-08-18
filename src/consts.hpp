#ifndef GPKIH_CONSTS
#define GPKIH_CONSTS

#pragma once

/* Misc */
#ifdef _WIN32
#include <Windows.h>
constexpr const char *vpnConfigExtension = "ovpn";
constexpr char SLASH = '\\';
#else
constexpr const char *vpnConfigExtension = "conf";
constexpr char SLASH = '/';
#endif

constexpr char EOL = '\n';

/* Custom return codes */
constexpr int GPKIH_OK = 0;
constexpr int GPKIH_FAIL = 1;
constexpr int GPKIH_FATAL = 2;

/* Limits */
constexpr int GPKIH_MAX_PATH = 4096;
constexpr int GPKIH_MAX_VARCHAR = 254;

/* Config filenames */
constexpr const char *vpnConfFilename = "openvpn.conf";
constexpr const char *pkiConfFilename = "pki.conf";

#endif