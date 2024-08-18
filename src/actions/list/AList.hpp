#pragma once

#include "../iface.hpp"
#include "../../libs/printing/printing.hpp"

class AList : public IAction 
{
protected:
	const char *usage() override;
	const char *examples() override;
	
public:
	AList(std::vector<std::string> &args):IAction(args){};
	AList() = delete;
	AList(AList&) = delete; 
	int exec() override;
};