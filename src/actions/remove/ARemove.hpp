#include "../iface.hpp"

class ARemove : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	ARemove(std::vector<std::string> &args):IAction(args){};
	ARemove() = delete;
	ARemove(ARemove&) = delete; 
	int exec() override;
};