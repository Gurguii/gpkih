#include "../iface.hpp"

class ARename : public IAction 
{
private:
	ARename() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const ARename& get(){
		static ARename _singleton{};
		return _singleton;
	}
	ARename(ARename&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};