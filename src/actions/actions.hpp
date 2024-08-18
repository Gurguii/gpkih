#include "iface.hpp"
#include <memory>
#include <optional>

namespace gpkih::actions
{
	extern std::optional<std::unique_ptr<IAction>> GetAction(std::string_view name, std::vector<std::string> &args);
}