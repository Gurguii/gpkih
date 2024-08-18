#include "../iface.hpp"

class AInit : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	AInit(std::vector<std::string> &args):IAction(args){};
	AInit() = delete;
	AInit(AInit&) = delete; 
	int exec() override;
};