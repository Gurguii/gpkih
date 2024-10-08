#include <libpq-fe.h>
#include <openssl/asn1.h>
#include <openssl/pem.h>
#include <openssl/x509.h>
#include "libs/ssl/pkey/rsa.hpp"
#include "libs/ssl/pkey/ed25519.hpp"
#include <string_view>
#include <unordered_map>
#include <string>

class X509Cert{
private:
	X509* _cert = nullptr;
	
	std::unordered_map<std::string, const EVP_MD*> _hashMapping
	{
		{"sha1",EVP_sha1()},
		{"md5",EVP_md5()}
	};

	std::string lastError{};
	unsigned char *cn;
	const char *location = nullptr; 
	const char *organisation = nullptr; 
	const char *email = nullptr; 
	const char *country = nullptr;;

public:
	X509Cert() = delete;
	
	X509Cert(unsigned char *cn, size_t serial, ISmartPKEY *key, size_t validityInSeconds):cn(cn){
		/* Initialize X509 structure */
		_cert = X509_new();
		
		/* Set common_name */
		X509_NAME *name;
		name = X509_get_subject_name(_cert);
		X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, cn, -1, -1, 0);
		X509_set_issuer_name(_cert, name);

		/* Set serial */
		ASN1_INTEGER_set(X509_get_serialNumber(_cert), serial);
		
		/* Set certificate validity */
		X509_gmtime_adj(X509_get_notBefore(_cert), 0);
		X509_gmtime_adj(X509_get_notAfter(_cert), validityInSeconds);

		/* Set pubkey */
		X509_set_pubkey(_cert, const_cast<EVP_PKEY*>(key->key()));
	}

	~X509Cert(){
		X509_free(_cert);
	}

	int sign(ISmartPKEY* key, std::string_view hash = "md5"){
		
		if(_hashMapping.find(hash.data()) == _hashMapping.end()){
			return -1;
		}

		if(!X509_sign(_cert, const_cast<EVP_PKEY*>(key->key()), _hashMapping[hash.data()])){
			return -1;
		};

		return 0;
	};

	int dump(std::string_view filePath){
		FILE* file = fopen(filePath.data(), "wb");
		
		if(file == nullptr){
			return -1;
		}

		return PEM_write_X509(file, _cert);
	}
};

int main()
{
	auto rsaKey = gssl::rsa::generateKeypair();
	X509Cert certificate((unsigned char*)"GurguiCA", 100, rsaKey, 3600 * 30);
	certificate.sign(rsaKey);
	certificate.dump("signed.pem");
	return 0;
}