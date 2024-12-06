#include "../x509/x509.hpp"
#include "../x509/req/req.hpp"
#include "../pkey/rsa.hpp"
#include "../pkey/ed25519.hpp"

#include <iostream>

int main()
{
	/* Generate CA & SV keypairs */
	ISmartPKEY* caKey = gssl::rsa::generateKeypair();

	X509Req csr(caKey, "ca", "gurguiCA", "ES");
	X509Cert crt((unsigned char*)"myCA", 1, caKey, 30);

	csr.dump();
	crt.dump();
	return 0;
}