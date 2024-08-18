#pragma once
#include "../iface.hpp"

class ANew : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	static inline const char *name = "new";
	ANew(std::vector<std::string> &args):IAction(args){};
	ANew() = delete;
	ANew(ANew&) = delete; 
	int exec() override;
};