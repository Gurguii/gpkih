#include "ASet.hpp"

constexpr const char *_name = "set";

constexpr const char *_usage = R"(== set ==
PARTIALLY IMPLEMENTED
set generic/profile-specific configuration

[ syntax ]
  ./gpkih set <profile> @vpn|pki.<section> key1=nval1 [...] @vpn|pki.<section> key1=nval1 [...] 
  ./gpkih set @<section> key1=nval2 [...] @<section> key1=nval1 [...]

note: when not giving a profile, 'gpkih' is assumed as the context so valid
sections will be sections in gpkih.conf.

[ subopts ]
  ** no subopts **

[ examples ]
  ./gpkih set @behaviour autoanswer=no headers=yes
  ./gpkih set myProfile @pki.output create_pfx=no create_inline=yes
  ./gpkih set myProfile @vpn.client remote="192.168.1.14 9999" @vpn.common cipher=AES-256-GCM
)";

constexpr const char *_examples = R"(== set examples ==
1. Change gpkih global configuration
  ./gpkih set behaviour.prompt=no
2. Change gpkih global configuration using scopes
  ./gpkih set @behaviour prompt=no autoanswer=yes
3. Change profile configuration
  ./gpkih set myProfile vpn.common.proto=tcp
4. Change profile configuration using scopes
  ./gpkih set myProfile @pki.key algorithm=rsa size=2048

)";

const char *ASet::usage(){
	return _usage;
};

const char *ASet::examples(){
	return _examples;
}