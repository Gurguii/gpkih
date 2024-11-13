#pragma once

#include <openssl/evp.h>
#include <unordered_map>

enum class PKEY_TYPE{
	rsa,
	ed25519,
//	ec,
//	dsa,
//	dh,
//	x25519,
//	x448,
//	sm2,
};

namespace gssl::ciphers{
	enum class Algorithm {
		none,
		AES_128_CBC,
		AES_256_CBC
	};
	extern std::unordered_map<Algorithm, const EVP_CIPHER*> available_ciphers;
}