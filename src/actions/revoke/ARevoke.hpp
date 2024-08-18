#include "../iface.hpp"

class ARevoke : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	ARevoke(std::vector<std::string> &args):IAction(args){};
	ARevoke() = delete;
	ARevoke(ARevoke&) = delete; 
	int exec() override;
};