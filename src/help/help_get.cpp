#include "help.hpp"

using namespace gpkih;

void help::get::usage()
{
	fmt::print(R"(
== set ==

[ syntax ]
  ./gpkih get <profile> <file>.<section>.<property> 

note: when working with gpkih.conf, profile its not required since this
configuration its global, e.g '/.gpkih get behaviour'

[ files and sections ]
  pki   : key, csr, crt, subject
  vpn   : common, client, server

[ examples ]
  ./gpkih get test pki : print all pki configuration
  ./gpkih get test pki.key : print [key] section
  ./gpkih get test pki.key.size : print `size` property from [key] section
)");
}
// TODO - add some sort of 'file selector' e.g ./gpkih get test @pki key csr @vpn client | to avoid specifying the whole path scope, same could be done
// e.g ./gpkih get test @pki.key size algorithm

// ./gpkih get test pki : print all from pki.conf
// ./gpkih get test pki.key : print all [ key ] section from profile test's pki.conf
// ./gpkih get test pki.key.size : print key size from profile test's pki.conf