#include "../iface.hpp"

class ARemove : public IAction 
{
private:
	ARemove() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const ARemove& get(){
		static ARemove _singleton{};
		return _singleton;
	}
	ARemove(ARemove&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};