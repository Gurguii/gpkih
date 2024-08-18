#include "../iface.hpp"

class ASet : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	ASet(std::vector<std::string> &args):IAction(args){};
	ASet() = delete;
	ASet(ASet&) = delete; 
	int exec() override;
};