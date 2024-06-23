#include "../iface.hpp"

namespace gpkih::help::iHelper
{
  class remove : public IHelper
  {
  public:
    remove() = default;
    
    void usage(bool brief = false){
        fmt::print(R"(== remove ==
remove profile/s

[ syntax ]
 ./gpki remove <profile1>,<profile2>...<profileN> [subopts]

[ subopts ]
  -all : remove all profiles instead
)");
    }
  };
} // namespace 

