#ifndef gssl_ciphers
#pragma once

#include <openssl/evp.h>
#include <unordered_map>

namespace gssl::ciphers
{
	enum class Algorithm {
		none,
		AES_128_CBC,
		AES_256_CBC
	};
	extern std::unordered_map<Algorithm, const EVP_CIPHER*> available_ciphers;
};

#endif