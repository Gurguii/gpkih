#include "rsa.hpp"
#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/params.h>

/* struct RSAkeypair */
RSAkeypair::RSAkeypair(EVP_PKEY *key, int bits):ISmartPKEY(key),_keysize(bits){};
/* struct RSAkeypair */

[[nodiscard("Returned PTR must be manually freed")]]
RSAkeypair* gssl::rsa::generateKeypair(int bits){
	auto ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, nullptr);
	
	if(ctx == nullptr){
		fprintf(stderr, "Couldn't initialize key context for RSA\n");
		return nullptr;
	}

	if(EVP_PKEY_keygen_init(ctx) <= 0){
		fprintf(stderr, "EVP_PKEY_keygen_init()\n");
		return nullptr;
	}

	if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, bits) <= 0){
		fprintf(stderr, "EVP_PKEY_CTX_set_rsa_keygen_bits()\n");
		return nullptr;
	}

	EVP_PKEY *key = nullptr;
	EVP_PKEY_keygen(ctx, &key);
	EVP_PKEY_CTX_free(ctx);

	auto pt = new RSAkeypair{key, bits};
	
	return pt;
};

int RSAkeypair::keySize(){
	return _keysize;
}

std::pair<const unsigned char *,size_t>RSAkeypair::encrypt(const unsigned char *data, size_t dataSize, const unsigned char *iv, gssl::ciphers::Algorithm algo){
	// TODO - implement IV usage if given
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_from_pkey(nullptr, const_cast<EVP_PKEY*>(key()), nullptr);
	if(ctx == nullptr){
		return {nullptr,0};
	}
	if(EVP_PKEY_encrypt_init(ctx) <= 0){
		EVP_PKEY_CTX_free(ctx);
		return {nullptr,0};
	};

	if(iv != nullptr && algo != gssl::ciphers::Algorithm::none){
		printf("unimplemented IV usage in RSAkeypair::encrypt()\n");
		return {nullptr, 0};
	}

	size_t buffLen = 0;
	if(EVP_PKEY_encrypt(ctx, nullptr, &buffLen, reinterpret_cast<const unsigned char*>(data), dataSize) <= 0){
		EVP_PKEY_CTX_free(ctx);
		return {nullptr,0};
	};
	unsigned char *buffer = (unsigned char*)malloc(buffLen);

	if(EVP_PKEY_encrypt(ctx, buffer, &buffLen, reinterpret_cast<const unsigned char*>(data), dataSize) <= 0){
		EVP_PKEY_CTX_free(ctx);
		free(buffer);
		return {nullptr,0};
	};

	EVP_PKEY_CTX_free(ctx);
	return {buffer,buffLen};
}

std::pair<const unsigned char*,size_t>RSAkeypair::decrypt(const unsigned char* data, size_t dataSize, const unsigned char *iv, gssl::ciphers::Algorithm algo){
	// TODO - implement IV usage if given
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_from_pkey(nullptr, const_cast<EVP_PKEY*>(key()), nullptr);
	if(ctx == nullptr){
		return {nullptr,0};
	}
	if(EVP_PKEY_decrypt_init(ctx) <= 0){
		EVP_PKEY_CTX_free(ctx);
		return {nullptr,0};
	};
	size_t buffLen = 0;
	if(EVP_PKEY_decrypt(ctx, nullptr, &buffLen, data, dataSize) <= 0){
		EVP_PKEY_CTX_free(ctx);
		return {nullptr,0};
	}

	unsigned char *buffer = (unsigned char*)malloc(buffLen);
	if(EVP_PKEY_decrypt(ctx, buffer, &buffLen, data, dataSize) <= 0){
		free(buffer);
		EVP_PKEY_CTX_free(ctx);
		return {nullptr,0};
	}

	EVP_PKEY_CTX_free(ctx);
	return {buffer, buffLen};
}