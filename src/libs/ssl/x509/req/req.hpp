#ifndef gssl_req
#define gssl_req
#pragma once
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include "../../pkey/ifaces.hpp"

// entityType: <client,server,ca>
class X509Req
{
private:
	ISmartPKEY *const _pkey;
	const char *_cn;
	std::unique_ptr<X509_REQ, decltype(&X509_REQ_free)> _req;
public:
	X509Req() = delete;

	X509Req(ISmartPKEY *pkey,
		std::string_view entityType,
		std::string_view cn,
		std::string_view country = "", 
		std::string_view location = "", 
		std::string_view organisation = "", 
		std::string_view state = ""
	);

	/* Not implemented */
	//X509Req(ISmartPKEY *pkey,
	//	const char *entityType,
	//	const char *cn,
	//	const char *algorithm = "ed25519",
	//	uint bits = 256,
	//	std::string_view country = "", 
	//	std::string_view location = "", 
	//	std::string_view organisation = "", 
	//	std::string_view state = ""
	//);

	explicit X509Req(ISmartPKEY *pkey);

	FILE *dump(FILE *outFile = stdout);
	int dump(std::string_view outFile);
};
#endif