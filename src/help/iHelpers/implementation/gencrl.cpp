#include "../iface.hpp"


namespace gpkih::help::iHelper
{
  class gencrl : public IHelper
  {
  public:
    gencrl() = default;
    void usage(bool brief = false) {
      fmt::print(R"(== gencrl ==
generate new certificate revokation list for profile
[ syntax ]
  ./gpki gencrl <profile> [subopts]
[ subopts ]
  *no subopts yet*
    )");
    }
  }; // class Gencrl
} // namespace 










