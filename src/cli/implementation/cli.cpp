#include "../cli.hpp"
#include "../../parse/parser.hpp"
#include "../../libs/utils/utils.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../config/Config.hpp"
#include <iostream>

namespace gpkih::cli::commands
{
#ifdef _WIN32
	constexpr const char* clear_console = "cls";
#else
	constexpr const char* clear_console = "clear";
#endif
}

std::string _customPS{};
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

static std::unordered_map <std::string, std::string(*)()>& placeholderMap()
{
	// This way if it ever gets called multiple times (e.g changing cli layout while in cli mode)
	// it wont create another map, but it wont get allocated unless called
	static std::unordered_map <std::string, std::string(*)()> _map{
		{"pn", []() {return std::string{"gpkih"};}},
		{"path", []() {return fs::current_path().string();}},
		#ifdef _WIN32
		{"u",[]() {return gpkih::utils::env::get_environment_variable("USERNAME");}},
		#else
		{"u",[]() {return gpkih::utils::env::get_environment_variable("USER");}},
		#endif
	};

	return _map;
};

static std::unordered_map<std::string, std::string(*)()>& inputPlaceholderMap(){
	static std::unordered_map<std::string, std::string(*)()> _map{
		#ifdef _WIN32
		{"~",[](){return gpkih::utils::env::get_environment_variable("USERPROFILE");}},
		#else
		{"~",[]() {return gpkih::utils::env::get_environment_variable("HOME");}},
		#endif
	};
	return _map;
};

static inline void __set_active_profile(const char* profile_name) {
	_customPS = customPS;
	active_profile = profile_name;
	customPS = std::move(fmt::format("{}{}", fmt::format(fg(PALE_GOLDEN_ROD) | EMPHASIS::bold, "[{}] ", profile_name), std::move(customPS)));
};

static inline void __reset_active_profile(){
	active_profile.assign("");
	customPS = _customPS;
}

static inline void __render_customPS() {
	fwrite(&customPS[0], 1, customPS.size(), stdout); // write customPS to console
}

static inline void __setup_PS() {
	// Load configuration from gpkih.conf 
	// TODO - add customizable style for profile
	// map strings to custom COLOR or STYLE from printing.hpp
	auto syntax = Config::get("cli", "customPS");

	if (syntax.empty() || std::count(syntax.begin(), syntax.end(), '&') & 1) {
		// & count is not even, which means there isn't a closing & for each opening & => wrong
		customPS = fallback_PS();
		return;
	}

	std::string ps{};
	auto color_map = map_str_color();
	auto phMap = placeholderMap();
	for (auto iter = syntax.begin(); iter != syntax.end(); ++iter) {
		char c = *iter;
		if (c == '&') {
			std::string placeholder{};
			// iterate with ++iter to pass the opening '&'
			while (++iter != syntax.end() && *iter != '&') {
				placeholder += *iter;
			}
			++iter; // pass the closing '&'
			if (phMap.find(placeholder) != phMap.end()) {
				ps += std::move(phMap[placeholder]());
				placeholder.assign("");
				continue;
			}
			else if (color_map.find(placeholder) != color_map.end()) {
				// Look for RESET or until syntax.end() is reached
				std::string coloured_string{};
				for (;iter != syntax.end(); ++iter) {
					if (*iter == '&') {
						std::string token{};
						while (++iter != syntax.end() && *iter != '&') {
							token += *iter;
						}
						if (phMap.find(token) != phMap.end()) {	
							coloured_string += std::move(phMap[token]());
							continue;
						}
						else if (token == "RESET") {
							ps += std::move(fmt::format(fg(color_map[placeholder.c_str()]), std::move(coloured_string)));
							placeholder.assign("");
							break;
						}
					}
					else {
						coloured_string += *iter;
					}
				}
				if (iter == syntax.end()) {
					PWARN("found color placeholder '{}' without matching RESET\n", placeholder);
					ps += std::move(coloured_string);
					break;
				}
			}
		}
		else {
			ps += c;
		}
	}

	customPS = std::move(ps);
}

void cli::init() {
	PDEBUG(1,"initializing cli");
	__setup_PS();
	PDEBUG(1,"PS settled up");
	std::string user_input;
	auto placeholderMap = inputPlaceholderMap();
	for (;;)
	{
		memset(&user_input[0], 0, user_input.size()); // clear user_input
		fwrite(&customPS[0], 1, customPS.size(), stdout); // print PS
		getline(std::cin, user_input);

		std::stringstream ss;

		auto pos = user_input.find('~');

		if(pos != -1){
			std::string new_user_input = "";
			for(int i = 0; i < pos; ++i){
				new_user_input += user_input[i];
			}
			new_user_input += placeholderMap["~"]();
			for(int i = pos+1; i < user_input.size(); ++i){
				new_user_input += user_input[i];
			}
			ss = std::stringstream(new_user_input);
		}else{
			ss = std::stringstream(user_input);
		}

		std::string action;
		ss >> action;

		char first = action[0];

		if (first == '@') {
			if(action.size() == 1){
				// switch back to no profile scope
				__reset_active_profile();
				continue;
			}
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

		if( parsers::parseGlobals(opts) == GPKIH_FAIL
			|| parsers::parse(opts) == GPKIH_FAIL){
			continue;
		}

		// call action's parser which will do the rest
		ACTION_PARSERS[action](opts);
	}
}