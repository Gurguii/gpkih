#pragma once
#include "../iface.hpp"

class AExport : public IAction 
{
private:
	AExport() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AExport& get(){
		static const AExport _singleton{};
		return _singleton;
	};
	AExport(AExport&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};