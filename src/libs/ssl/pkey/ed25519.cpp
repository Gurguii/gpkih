#include "ed25519.hpp"
#include <openssl/evp.h>


ED25519keypair::ED25519keypair(EVP_PKEY *key):ISmartPKEY(key){}

using namespace gssl;

ED25519keypair* ed25519::generateKeypair(){
	EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_ED25519, nullptr);
	if(ctx == nullptr){
		return nullptr;
	}
	if(EVP_PKEY_keygen_init(ctx) <= 0){
		return nullptr;
	};
	EVP_PKEY *key = nullptr;
	if(EVP_PKEY_keygen(ctx, &key) <= 0){
		return nullptr;
	}
	auto kp = new ED25519keypair{key};
	return kp;
}