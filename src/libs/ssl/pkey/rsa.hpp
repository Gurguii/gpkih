#pragma once
#include <openssl/pem.h>
#include "ifaces.hpp"
#include "ciphers.hpp"

#include <utility>

struct RSAkeypair : public ISmartPKEY{
private:
	int _keysize; 
public:
	explicit RSAkeypair(EVP_PKEY *key, int bits);
	int keySize();
	
	std::pair<const unsigned char *,size_t>
	encrypt(const unsigned char *data, size_t dataSize, const unsigned char *iv = nullptr, gssl::ciphers::Algorithm algo = gssl::ciphers::Algorithm::none);
	
	std::pair<const unsigned char *,size_t>
	decrypt(const unsigned char *data, size_t dataSize, const unsigned char *iv = nullptr, gssl::ciphers::Algorithm algo = gssl::ciphers::Algorithm::none);
};

namespace gssl::rsa
{
	RSAkeypair* generateKeypair(int bits = 2048);
}