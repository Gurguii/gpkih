#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include "../../pkey/ifaces.hpp"

class X509Req
{
private:
	ISmartPKEY *const pkey;
	X509_REQ *req = nullptr;
public:
	X509Req() = delete;
	X509Req(std::string_view algorithm, int bits);
	explicit X509Req(ISmartPKEY *pkey);
};