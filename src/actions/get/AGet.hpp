#include "../iface.hpp"

class AGet : public IAction 
{
private:
	AGet() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AGet& get(){
		static AGet _singleton{};
		return _singleton;
	}
	AGet(AGet&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};