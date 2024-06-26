#include "../iface.hpp"

namespace gpkih::help::iHelper
{
class init : public IHelper{
  public:
    init() = default;
    void usage(bool brief = false){
      fmt::print(R"(== init ==
Create a new profile with its according PKI file structure

[ syntax ]
  ./gpki init [subopts]

[ subopts ]
  -n | --name : profile name
  -s | --source : source dir (where profile related files will be kept)"
    );
  }
};
}