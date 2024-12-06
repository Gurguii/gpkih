#ifndef gssl_ed25519
#define gssl_ed25519

#pragma once
#include "ifaces.hpp"
#include <openssl/pem.h>

struct ED25519keypair : public ISmartPKEY{
public:
	explicit ED25519keypair(EVP_PKEY *key);
};

namespace gssl::ed25519{
	ED25519keypair* generateKeypair();
}

#endif