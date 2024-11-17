#include <openssl/evp.h>

#include <memory>
#include <string>

namespace gssl::dhparam
{

class DHparam {
private:
    std::unique_ptr<EVP_PKEY, decltype(&EVP_PKEY_free)> _pkey;
    std::string _lastError;

public:
    explicit DHparam(EVP_PKEY *pkey);

    /* BEG - Getters */
    const std::string& lastError() const;
    const EVP_PKEY* const pkey() const;
    /* END - Getters */

    /* BEG - Methods */
    // return 0:ok 1:fail 
    int dump(FILE *outFile = stdout, std::string_view outFormat = "PEM");
    /* END - Methods */
};

DHparam* generate(unsigned int pbits = 2048, std::string_view hash = "sha256");
} // namespace gssl::dhparam