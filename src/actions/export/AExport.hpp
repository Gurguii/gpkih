#pragma once
#include "../iface.hpp"

class AExport : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
public:
	AExport(std::vector<std::string> &args):IAction(args){};
	AExport() = delete;
	AExport(AExport&) = delete; 
	int exec() override;
};