#include <openssl/evp.h>
#include <string_view>

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
	int dump(FILE *outFile = stdout, std::string_view outFormat = "PEM");
};

namespace gssl::dhparam
{
	extern DHparam* generate(unsigned int pbits = 2048, std::string_view digest = "sha256");
}