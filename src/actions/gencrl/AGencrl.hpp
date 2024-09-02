#include "../iface.hpp"

class AGencrl : public IAction 
{
private:
	AGencrl() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AGencrl& get(){
		static AGencrl _singleton{};
		return _singleton;
	}
	AGencrl(AGencrl&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};