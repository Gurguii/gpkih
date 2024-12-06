#ifndef gssl_x509
#define gssl_x509

#pragma once
#include "../x509/req/req.hpp"

#include <openssl/evp.h>

#include <string_view>
#include <unordered_map>
#include <string>

class X509Cert{
private:
	X509* _cert = nullptr;
	
	std::unordered_map<std::string, const EVP_MD*> _hashMapping
	{
		{"sha1",EVP_sha1()},
		{"sha3_256",EVP_sha3_256()},
		{"sha3_512",EVP_sha3_512()},
		{"md5",EVP_md5()},
		{"sha256",EVP_sha256()},
		{"sha512",EVP_sha512()}
	};

	std::string lastError = "no error";

	/* mandatory */
	unsigned char *cn = nullptr;
	size_t serial = 0;
	
	/* optional */
	unsigned char *location = nullptr; 
	unsigned char *organisation = nullptr; 
	unsigned char *email = nullptr; 
	unsigned char *country = nullptr;

public:
	X509Cert();

	/* Self-signed */
	X509Cert(unsigned char *cn, size_t serial, ISmartPKEY *key, size_t days);
	~X509Cert();
	std::string &getError();
	X509* const getCertificate();
	
	/* 
		RETURN CODES
	    1 - success
	    0 - failure - call getError() for further info
	*/

	/* signing */
	int sign(ISmartPKEY* key, std::string_view hash = "md5");

	/* dumping */
	int dump(std::string_view filePath);
	int dump(FILE *file = stdout);
	int dump2(std::string_view filePath, std::string_view format = "pem");

	/* setting subject info */
	int set_serial();
	int set_cn();
	int set_key();

	/* getting subject info */
	size_t get_serial();
	unsigned char *get_cn();

	/* x509v3 extensions -UNIMPLEMENTED- */
	int add_ext();
	int del_ext();
};

#endif