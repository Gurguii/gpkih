#include "../iface.hpp"

namespace gpkih::help::iHelper
{
  class revoke : public IHelper
  {
  public:
    revoke() = default;
    void usage(bool brief = false){
  fmt::print(R"(== revoke ==
revoke entitie/s

[ syntax ]
  ./gpki revoke <profile> [subopts]
  
[ subopts ]
  -s  | --serial : entity serial to revoke
  -cn | --cn     : entity common name to revoke
)");
    }
  };
} // namespace