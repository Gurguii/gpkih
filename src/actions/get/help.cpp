#include "AGet.hpp"

constexpr const char *_usage = R"(== get ==
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
)";

constexpr const char *_examples = R"(== get examples ==
1. Print all PKI configuration from profile
  ./gpkih get myProfile pki
2. Print profile KEY section from PKI configuration
  ./gpkih get myProfile pki.key
3. Print profile KEY ALGORITHM used for new entities
  ./gpkih get myProfile pki.key.size
4. Print using scope resolution for less typing
  ./gpkih get myProfile @pki key subject
5. Print using scope resolution on section
  ./gpkih get myProfile @pki.key algorithm size
)";

const char *AGet::usage(){
	return _usage;
};

const char *AGet::examples(){
	return _examples;
}