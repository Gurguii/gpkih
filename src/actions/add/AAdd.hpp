#pragma once
#include "../iface.hpp"

class AAdd : public IAction 
{
private:
	AAdd() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AAdd& get(){
		static AAdd _singleton{};
		return _singleton;
	}
	AAdd(AAdd&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};