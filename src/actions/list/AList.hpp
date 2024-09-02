#pragma once

#include "../iface.hpp"

class AList : public IAction 
{
private:
	AList() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AList& get(){
		static AList _singleton{};
		return _singleton;
	}
	AList(AList&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};