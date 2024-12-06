#include "ciphers.hpp"

using namespace gssl;

std::unordered_map<ciphers::Algorithm, const EVP_CIPHER*> ciphers::available_ciphers = {
	{ciphers::Algorithm::AES_128_CBC,EVP_aes_128_cbc()},
	{ciphers::Algorithm::AES_256_CBC,EVP_aes_256_cbc()}
};