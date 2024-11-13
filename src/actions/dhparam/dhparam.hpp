#pragma once
#include "../iface.hpp"

class ADHparam : public IAction 
{
private:
	ADHparam() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const ADHparam& get(){
		static ADHparam _singleton{};
		return _singleton;
	}
	ADHparam(ADHparam&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};