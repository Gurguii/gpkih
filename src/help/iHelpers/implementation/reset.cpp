#include "../iface.hpp"

namespace gpkih::help::iHelper
{
	class reset : public IHelper
	{
	public:
		reset() = default;
		void usage(bool brief = false){
			fmt::print(R"(== reset ==
reset gpkih, removing every profile and gpkih source dir, where database and logs are located

[ syntax ]
./gpkih reset
)");
		}
	};
} // namespace