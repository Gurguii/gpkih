#include "../iface.hpp"

class AGet : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	AGet(std::vector<std::string> &args):IAction(args){};
	AGet() = delete;
	AGet(AGet&) = delete; 
	int exec() override;
};