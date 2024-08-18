#include "../iface.hpp"

class AReset : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	AReset(std::vector<std::string> &args):IAction(args){};
	AReset() = delete;
	AReset(AReset&) = delete; 
	int exec() override;
};