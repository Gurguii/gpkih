#pragma once
#include <string>
#include <unordered_map>
#include "config/config_management.hpp"

// This file is unused, this is just a 
// concept of something I might implement
// for command line options' parsing 

struct gpkih_opt
{
	CONFIG_FILE file;
	std::string section;
	std::string key; 
};

static inline std::unordered_map<std::string,gpkih_opt> gpkih_opts
{
	/* gpkih.conf */
	{"-y",{CONFIG_PKI,"behaviour","autoanswer_yes"}},
	/* pki.conf */
	{"-keysize",{CONFIG_PKI,"key","size"}},
	/* openvpn.conf */
	{"--client-remote",{CONFIG_VPN,"client","remote"}}
};