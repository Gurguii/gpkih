#include "parser.hpp"

using namespace gpkih;



static void print_on_scope(strview config_file, strview section) 
{

}
static void print_on_scope(strview config_file) 
{

}
template <typename ...T> static void print_on_scope(T&& ...args) 
{

}
// get <profile> <config>.<section>.<prop>
// if no profile given - gpkih is used
// if @ is used, the context is changed until
// another @ is encountered or all args are 
// parsed, e.g 

// ./gpkih get test @pki key.size key.algorithm | @pki indicates the context (pki.conf) so instead of pki.key.size *.*.algorithm, u omit pki
// ./gpkih get test @vpn.client remote			| @vpn.client sets the current scope to vpn.conf section [client]

int parsers::get(std::vector<str> opts)
{
	if (opts.empty()) {
		// print gpkih general configuration
		Config::print();
		return GPKIH_OK;
	}

	// Check profile
	strview profile_name = opts[0];
	Profile profile;
	if (db::profiles::load(profile_name, profile)) {
		return GPKIH_FAIL;
	};
	opts.erase(opts.begin());

	// Profile exists, load its config
	ProfileConfig config(profile);
	if (config.succesfully_loaded != GPKIH_OK) {
		return GPKIH_FAIL;
	}

	// 
	std::unordered_map<str, ConfigMap*> scopes
	{
		{"@pki", config.get(CONFIG_PKI)},
		{"@vpn", config.get(CONFIG_VPN)},
		{"@gpkih", Config::get()},
	};

	ConfigMap* current_scope = nullptr;
	// Iterate on args and print requested configuration
	for (int i = 0; i < opts.size(); ++i) {
		strview opt = opts[i];
		if (opt[0] == '@') {
			auto dot_finder = std::find(opt.begin(), opt.end(), '.');
			str name(opt.begin(), dot_finder);
			// got a scope resolution to do
			int dots = std::count(opt.begin(), opt.end(), '.');
			switch (dots) {
				case 0:
					// ./gpkih get test @pki
					break;
				case 1:
					// ./gpkih get test @pki.key
					break;
				default:
					// Warn and ignore
					PWARN("scope resolution too deep on '{}' max depth: 2\n", opt);
					break;
			}
		}
	}
	return GPKIH_OK;
}