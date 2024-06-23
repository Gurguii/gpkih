#include "../iface.hpp"

namespace gpkih::help::iHelper
{
  class rename : public IHelper
  {
  public:
    rename() = default;
    void usage(bool brief = false){
      fmt::print(R"(== rename ==
rename profile
[ syntax ]
 ./gpki rename <profile> <newname> [subopts]
[ subopts ]
  * no specific subopts added * 
)");
}  
};

} // namespace