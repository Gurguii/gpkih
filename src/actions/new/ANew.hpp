#pragma once
#include "../iface.hpp"

class ANew : public IAction 
{
private:
	ANew() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const ANew& get(){
		static ANew _singleton{};
		return _singleton;
	}
	ANew(ANew&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};