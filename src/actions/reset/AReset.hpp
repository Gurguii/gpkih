#include "../iface.hpp"

class AReset : public IAction 
{
private:
	AReset() = default;
protected:
	const char *usage() const override;
	const char *examples() const override;
public:
	static const AReset& get(){
		static AReset _singleton{};
		return _singleton;
	}
	AReset(AReset&) = delete; 
	int exec(std::vector<std::string> &args) const override;
};