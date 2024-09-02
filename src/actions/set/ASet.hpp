#include "../iface.hpp"

class ASet : public IAction 
{
private:
	ASet() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const ASet& get(){
		static ASet _singleton{};
		return _singleton;
	}
	ASet(ASet&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};