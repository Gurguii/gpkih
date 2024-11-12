#include <openssl/evp.h>
#include <string_view>


/* 
   - Openssl supported DH implementations -
	* ffdhe3072, ffdhe4096, ffdhe6144, ffdhe8192, modp_2048, modp_3072, modp_4096, modp_6144, modp_8192
   - Openssl output options -
	* TEXT,PEM,DER
*/

struct DHparam
{
private:
	EVP_PKEY* _pkey;
	const char *_lastError = nullptr;
public:
	DHparam(EVP_PKEY *pkey);
	~DHparam();
	const char *lastError();
	const EVP_PKEY* const pkey();
	
	// 0:ok 1:fail
	int dump(std::string_view outPath, std::string_view outFormat = "PEM");
};

namespace gssl::dhparam
{
	extern DHparam* generate(std::string_view dh_implementation = "modp_2048", const char *digest = "sha256");
}