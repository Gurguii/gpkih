#include "../iface.hpp"

namespace gpkih::help::iHelper
{
class build : public IHelper
{
public:
  build() = default;
  
  void usage(bool brief = false){
    fmt::print(R"(== build ==
build certificates for a profile

[ syntax ]
  ./gpki build <profile> <entity_type> [subopts]
  
[ entity types ]
  ca : ca
  sv : server
  cl : client

[ subopts ]
  
  ** openssl **
  -algo    | --algorithm  <rsa,ed25519>      : algorithm to use
  -keysize | --keysize    <int>              : key size to use
  -outformat              <pem|der>          : output format
  -keyformat              <pem|der>          : key output format
  
  ** subject **
  -cn     | --common-name   <char[254]>   : set cn
  -serial | --serial        <char[254]>   : set serial
  -loc    | --location      <char[254]>   : set location
  -co     | --country       <char[254]>   : set country (2 letter code)
  -org    | --organisation  <char[254]>   : set organisation
  -st     | --state         <char[254]>   : set state
  -email  | --email         <char[254]>   : set email

  ** output **
  -inline | --inline : create inline file
  -pfx    | --pfx    : create pfx file (pkcs12), *will get prompted for password
)");
}
};
} // namespace gpkih::help::iHelper