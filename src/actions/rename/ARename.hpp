#include "../iface.hpp"

class ARename : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	ARename(std::vector<std::string> &args):IAction(args){};
	ARename() = delete;
	ARename(ARename&) = delete; 
	int exec() override;
};