#pragma once
#include <openssl/evp.h>
#include <string_view>

struct ISmartPKEY{
private:
	EVP_PKEY *_key = nullptr;
public:
    explicit ISmartPKEY(EVP_PKEY *key);
    ~ISmartPKEY(); // checks if _key is nullptr, if its not, it does EVP_PKEY_free(_key);
	
    const EVP_PKEY *const key();
    
    FILE *pem_dump_public(FILE *file);
	// return -1 on failure 0 on success
    int pem_dump_public(std::string_view filePath);

    FILE *pem_dump_private(FILE *file);
    // return -1 on failure 0 on success
    int pem_dump_private(std::string_view filePath);

    ISmartPKEY() = delete;
    ISmartPKEY(ISmartPKEY&) = delete;
    ISmartPKEY operator=(ISmartPKEY &ro) = delete;
};