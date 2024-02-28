#include "parser.hpp"

using namespace gpkih;

// get <profile> <config>.<section>.<prop>
// if no profile given - gpkih is used
// if @ is used, the context is changed until
// another @ is encountered or all args are 
// parsed, e.g 

// ./gpkih get test @pki key.size key.algorithm | @pki indicates the context (pki.conf) so instead of pki.key.size *.*.algorithm, u omit pki
// ./gpkih get test @vpn.client remote			| @vpn.client sets the current scope to vpn.conf section [client]
// ./gpkih get		@behaviour					| automaticly the context is 'gpkih' so using scope resolutions already treat it as gpkih.

using Formatter = gpkih::experimental::Formatter;
using FormatInfo = gpkih::experimental::FormatInfo;

static int __handle_gpkih_scope(std::vector<str> &opts)
{
	return GPKIH_OK;
};

static int __handle_scope_printing(ProfileConfig &config, std::vector<str>::iterator current, std::vector<str>::iterator end)
{
	return GPKIH_OK;
};

// Prints every section | keyval from `gpkih.conf`
static void __handle_gpkih_full_printing()
{
	auto finfo = Formatter::gpkih_formatinfo();

	Formatter formatter(finfo);
	
	ConfigMap& conf = *Config::get();

	for (const auto& section : conf) {
		formatter.print_headers(section.first);
		for (const auto& keyval : section.second) {
			formatter.print_keyval(keyval.first, keyval.second);
		}
	}
}

int parsers::get(std::vector<str> opts)
{
	if (opts.empty()) {
		// print gpkih general configuration
		__handle_gpkih_full_printing();
		return GPKIH_OK;
	}

	// Check profile
	strview profile_name = opts[0];

	Profile profile;
	if (db::profiles::load(profile_name, profile)) {
		return __handle_gpkih_scope(opts);
		// profile does not exist but we got more opts
		// the user is trying to get general config props??
		return GPKIH_FAIL;
	};
	opts.erase(opts.begin());

	// Profile exists, load its config
	ProfileConfig config(profile);
	if (config.succesfully_loaded != GPKIH_OK) {
		return GPKIH_FAIL;
	}
	
	// Iterate on args and print requested configuration
	//experimental::Formatter format(experimental::Formatter::FormatInfo{10, C_ALLIGN});
	for (int i = 0; i < opts.size(); ++i) {
		strview opt = opts[i];
		if (opt[0] == '@') {
			__handle_scope_printing(config, opts.begin() + i, opts.end());
			continue;
		}

		// No starting @ in the opt, get file - section - property
		// and print accordingly
		auto ndots = std::count(opt.begin(), opt.end(), '.');
		switch (ndots) {
		case 0:
			// full file
			if (opt == "pki") {
				config.print(CONFIG_PKI);
			}
			else if (opt == "vpn") {
				config.print(CONFIG_VPN);
			}
			break;
		case 1:
			// full section from file
			break;
		case 2:
			// specific property from file - section
			break;
		default:
			PWARN("scope resolution 2 deep\n");
			break;
		}
	}
	return GPKIH_OK;
}