#include "cli.hpp"
#include "../parse/parser.hpp" // ACTION_PARSERS - map contaning actio names and appropiate parse function ptr e.g {"build", gpkih::parse::build} 
#include "../utils/utils.hpp"
#include "../printing/printing.hpp"

#include <string>
#include <cstring>
#include <cstdio>
#include <unordered_map>
#include <sstream> // std::stringstream
#include <iostream> // std::cin

namespace gpkih::cli::commands
{
#ifdef _WIN32
	constexpr const char* clear_console = "cls";
#else
	constexpr const char* clear_console = "clear";
#endif
}

std::string customPS{};
static inline std::vector<const char*> command_history{};
std::string active_profile{};

using namespace gpkih;

// gpkih <action> <profile> <subopts>
// 1. on cli mode 'gpkih' is assumed
// 2. on cli mode, you can do @<profile> to 'select' the profile,
// so e.g without cli mode and @<profile> -> ./gpkih get foo vpn.client
// with cli mode:
// gpkih> @foo
// [foo] gpkih> get

static inline const char *fallback_PS(){
	return "gpkih >";
}

static std::unordered_map <std::string, std::string(*)() > ph_map
{
	{"pn", []() {return std::string{"gpkih"};}},
	{"path", []() {return fs::current_path().string();}},
#ifdef _WIN32
	{"u",[]() {return gpkih::utils::env::get_environment_variable("USERNAME");}},
#else
	{"u",[]() {return gpkih::utils::env::get_environment_variable("USER");}},
#endif
};

static inline void __set_active_profile(const char* profile_name) {
	active_profile = profile_name;
	customPS = std::move(fmt::format("{}{}", fmt::format(fg(LGREEN) | EMPHASIS::bold, "[{}] ", profile_name), std::move(customPS)));
};

static inline void __render_customPS() {
	fwrite(&customPS[0], 1, customPS.size(), stdout); // write customPS to console
}

static inline void __setup_PS() {
	// Load configuration from gpkih.conf 
	// TODO - add customizable style for profile
	// map strings to custom COLOR or STYLE from printing.hpp
	auto syntax = Config::get("cli", "customPS");

	if(syntax.empty()){
		customPS = fallback_PS();
		return;
	}

	std::stringstream ss(syntax.data());

	std::string ps{};
	auto color_map = map_str_color();

	for (auto iter = syntax.begin(); iter != syntax.end(); ++iter) {
		char c = *iter;
		if (c == '&') {	
			if (++iter == syntax.end()) {
				break;
			}
			
			std::string ph{};
			
			while (iter != syntax.end() && *iter != '&') {
				ph += *iter;
				++iter;
			}

			if (ph_map.find(ph) != ph_map.end()) {
				ps += std::move(ph_map[ph]());
			}
			else if (color_map.find(ph.c_str()) != color_map.end()) {
				ps += std::move(ph);
			}
		}
		else {
			ps += c;
		}
	}
	
	// Example: REDC:\testingRESET BLUEgurguiRESET@GREENgpkihRESET > 
	

	customPS = std::move(ps);
}

void cli::init() {

	__setup_PS();

	std::string user_input;
	
	for (;;)
	{
		memset(&user_input[0], 0, user_input.size()); // clear user_input
		fwrite(&customPS[0], 1, customPS.size(), stdout); // print PS
		getline(std::cin, user_input);

		std::string action;
		std::stringstream ss(user_input);
		ss >> action;

		char first = action[0];

		if (first == '@') {
			// select @<profile> as active profile
			// make it reflected in subsequent calls to 
			// parsers and in customPS
			if (!db::profiles::exists(&action[1])) {
				PWARN("profile {} doesn't exist\n", action);
				continue;
			}
			__set_active_profile(std::move(&action[1]));
			continue;
		}
		else if (first == '!') {
			// execute the user input as a system command
			const char* command = &user_input[1];
			system(command);
			continue;
		}
		if (action == "cls" || action == "clear") {
			// clear console
			system(cli::commands::clear_console);
			continue;
		}
		else if (action == "help" || action == "/?") {
			std::cout << "unimplemented cli help\n";
			continue;
		}
		else if (action == "exit" || action == "quit") {
			return;
		}
		// TODO - add checks for 'action' since it doesn't necesarily need to
		// be an action, e.g the user could input 'help' to get some help 
		// or exit to exit
		
		std::vector<str> opts{};
		std::string opt;
		
		if (!active_profile.empty()) {

			opts.push_back(active_profile);
		}

		while (ss >> opt) {
			opts.push_back(opt);
		}

		if (ACTION_PARSERS.find(action) == ACTION_PARSERS.end()) {
			PERROR("action '{}' doesn't exist\n", action);
			continue;
		}

		// call parser which will do the rest
		ACTION_PARSERS[action](opts);
	}
}

// TODO - develop this to have customizable PS
static inline std::unordered_map<std::string, std::string(*)()>ps_pholder_map
{
	{":hour:", []() {return std::string{"%h"}; }},
};