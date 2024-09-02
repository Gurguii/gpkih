#include "../iface.hpp"

class ARevoke : public IAction 
{
private:
	ARevoke() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const ARevoke& get(){
		static ARevoke _singleton{};
		return _singleton;
	}
	ARevoke(ARevoke&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};