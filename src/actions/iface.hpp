#pragma once

#include <vector>
#include <string>
#include <iostream>

/* 
	1- ./gpkih <action> <...> => IAction action(args); action.exec()
	2- ./gpkih help <action> <...> => IAction help(args); action.help()
*/

// TODO - Make IAction implementations singleton pattern. Allow default constructor w/o args and make the exec() function
// -> exec(std::vector<std::string> &args)

class IAction {
protected:
	std::vector<std::string> &args;
	virtual const char *usage() = 0;
	virtual const char *examples() = 0;
public:
	IAction(std::vector<std::string> &args):args(args){};
	virtual int exec() = 0;
	virtual ~IAction() = default;
	void help(){
		bool reqFullfilled = false;
		for(std::string &arg : args){
			if(arg == "-e" || arg == "--examples"){
				std::cout << examples() << "\n";
				reqFullfilled = true;
				break;
			}
		}
		!reqFullfilled && std::cout << usage() << "\n";
	};
};