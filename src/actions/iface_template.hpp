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
private:
	AName() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AName& get(){
		static AName _singleton{};
		return _singleton;
	}
	AName(AName&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};