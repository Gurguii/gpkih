#include "../x509/req/req.hpp"
#include "../pkey/ed25519.hpp"
#include "../pkey/rsa.hpp"

int main()
{
	printf("-- Starting with RSA\n");
	auto key = gssl::rsa::generateKeypair(); 
	key->pem_dump_private("key.pem");
	auto csr = X509Req(key, "client", "gurgui");
	csr.dump("rsa.req");

	printf("-- Starting with ED25519\n");
	auto key2 = gssl::ed25519::generateKeypair();
	key2->pem_dump_private("key2.pem");
	auto csr2 = X509Req(key2, "client", "gurgui");
	csr2.dump("ed25519.req");	
	
	return 0;
}