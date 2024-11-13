#include "ciphers.hpp"

using namespace gssl::ciphers;

std::unordered_map<Algorithm, const EVP_CIPHER*>available_ciphers = {
	{Algorithm::AES_128_CBC,EVP_aes_128_cbc()},
	{Algorithm::AES_256_CBC,EVP_aes_256_cbc()}
};