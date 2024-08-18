#include "iface.hpp"

/* 

This is a template used to add new actions, 
simply copy-paste this class to the action header, e.g
build.hpp, then just implement the virtual methods
with parse.cpp, help.cpp, and any other .cpp required to manage
the whole logic of the Action.

*/
class AName : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	AName(std::vector<std::string> &args):IAction(args){};
	AName() = delete;
	AName(AName&) = delete; 
	int exec() override;
};