#include "../iface.hpp"

class AInit : public IAction 
{
private:
	AInit() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AInit& get(){
		static AInit _singleton{};
		return _singleton;
	}
	AInit(AInit&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};