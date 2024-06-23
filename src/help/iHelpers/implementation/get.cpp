#include "../iface.hpp"

namespace gpkih::help::iHelper
{
  class get : public IHelper{
  public:
    get() = default;
    void usage(bool brief = false){
      fmt::print(R"(== get ==
get generic/profile-specific configuration

[ syntax ]
  ./gpkih get <profile> <file>.<section>.<property> 

note: when working with gpkih.conf, profile its not required since this
configuration its global, e.g './gpkih get behaviour'

[ files and sections ]
  gpkih : behaviour, logs, cli
  pki   : key, csr, crt, subject
  vpn   : common, client, server

[ examples ]
  ./gpkih get test pki : print all pki configuration
  ./gpkih get test pki.key : print [key] section
  ./gpkih get test pki.key.size : print `size` property from [key] section
      )");
    };
  };
} // namespace gpkih::help::iHelper