#include <vector>
#include <string_view>

namespace gpkih::sqlite
{
	extern int exportDB(std::string_view outDir, std::vector<std::string> &args);	
}