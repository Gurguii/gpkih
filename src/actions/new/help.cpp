#include "ANew.hpp"

constexpr const char *_usage = R"(== new ==
Add new entities

[ syntax ]
  ./gpki new <profile> <entity_type> [subopts]
  
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
)";

constexpr const char *_examples = R"(== new examples ==
  1. Add a CA
    ./gpkih new <myProfile> ca -cn <myCA>
  
  2. Add a SERVER avoiding user confirmation required
    ./gpkih new <myProfile> sv -cn <mySV> -country "ES" -location "Mi Casa" -y
  
  3. Add a CLIENT and create a .pfx file (will get prompted for password)
    ./gpkih new <myProfile> cl -cn <myCL> -pfx
  
  4. Add a CLIENT with all available options
    ./gpkih new <myProfile> cl -cn gurgui -co "ES" -lo "Gran Canarias" -org "Mariwanos" -mail "gurgui@mariwanos.com" -serial 200 -st "StateOfElevation" -pfx -algo rsa -keysize 2048 
)";

const char *ANew::usage() const {
	return _usage;
};

const char *ANew::examples() const {
	return _examples;
}