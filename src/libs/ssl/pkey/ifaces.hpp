#ifndef gssl_pkey_iface
#define gssl_pkey_iface

#pragma once
#include <openssl/evp.h>
#include <string_view>
#include <memory>

struct ISmartPKEY{
private:
    std::unique_ptr<EVP_PKEY,decltype(&EVP_PKEY_free)> _key;
public:
    explicit ISmartPKEY(EVP_PKEY *key);
    ~ISmartPKEY(); // checks if _key is nullptr, if its not, it does EVP_PKEY_free(_key);
	
    EVP_PKEY *key();
    
    FILE *pem_dump_public(FILE *file = stdout);
	// return -1 on failure 0 on success
    int pem_dump_public(std::string_view filePath);

    FILE *pem_dump_private(FILE *file = stdout);
    // return -1 on failure 0 on success
    int pem_dump_private(std::string_view filePath);

    ISmartPKEY() = delete;
    ISmartPKEY(ISmartPKEY&) = delete;
    ISmartPKEY operator=(ISmartPKEY &ro) = delete;
};

#endif