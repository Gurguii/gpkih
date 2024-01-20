#include "help.hpp"

using namespace gpki;

void help::generic::usage() {
  std::cout << R"(== Gurgui public key insfraestructure helper ==
[ Generic syntax ]
  ./gpki [action] [profile] [subopts]

== ACTIONS ==

[init]
  ./gpki init [subopts]
  subopts:
    -n | --name : profile name
    -s | --source : profile source dir (absolut path)
  examples:
    ./gpki init -n test -s /pki/test
    
[build] 
  ./gpki build [profile] [subopts]
  
  subopts:
    -ca | --ca     : create ca certificate
    -sv | --server : create server certificate
    -cl | --client : create client certificate
    -keysize | --keysize : set key size (default 2048)

  examples:
  - Build CA -
    ./gpki build profile1 -ca
  - Build Server -
    ./gpki build profile1 --server
  - Build Client -
    ./gpki build profile1 --client

[revoke]
  ./gpki revoke [profile] [subopts]

  subopts:
    -cn | --common-name <name> : entity to revoke
  
  examples:
   ./gpki revoke test -cn client1
  )";

}
