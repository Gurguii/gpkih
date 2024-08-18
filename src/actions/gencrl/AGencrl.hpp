#include "../iface.hpp"

class AGencrl : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	AGencrl(std::vector<std::string> &args):IAction(args){};
	AGencrl() = delete;
	AGencrl(AGencrl&) = delete; 
	int exec() override;
};