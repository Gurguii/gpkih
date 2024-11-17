#include "ssl/x509/req/req.hpp"
#include "ssl/pkey/ed25519.hpp"

int main()
{
	auto key = gssl::ed25519::generateKeypair(); 
	key->pem_dump_private("key.pem");
	auto csr = X509Req(key, "client", "gurgui");
	csr.dump("csr.pem");
	return 0;
}