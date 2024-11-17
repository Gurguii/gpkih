#include "dh.hpp"

#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/encoder.h>


namespace gssl::dhparam
{

	/* BEG - DHparam class */
DHparam::DHparam(EVP_PKEY *pkey):_pkey(pkey, EVP_PKEY_free){}
	
const std::string& DHparam::lastError() const{
	return _lastError;
}
	
const EVP_PKEY* const DHparam::pkey() const{
	return _pkey.get();
}
	
int DHparam::dump(FILE *outFile, std::string_view outFormat){
	    if (!outFile) {
	        _lastError = "Output file is null";
	        return 1;
	    }
	    
	    if (outFormat != "PEM" && outFormat != "DER" && outFormat != "TEXT") {
	        _lastError = "Invalid output format. Valid formats are: PEM, DER, TEXT";
	        return 1;
	    }
	    OSSL_ENCODER_CTX *ctx = OSSL_ENCODER_CTX_new_for_pkey(_pkey.get(), EVP_PKEY_KEY_PARAMETERS, outFormat.data(), nullptr, nullptr);
	    
	    if (!ctx) {
	        _lastError = "Failed initializing encoder context";
	        return 1;
	    }
	    
	    if (OSSL_ENCODER_to_fp(ctx, outFile) == 0) {
	        _lastError = "Failed writing to file";
	        OSSL_ENCODER_CTX_free(ctx);
	        return 1;
	    }
	
	    OSSL_ENCODER_CTX_free(ctx);
	    return 0;
	}
	/* END - DHparam class */

DHparam* generate(unsigned int pbits, std::string_view hash) {
    unsigned int qbits = pbits / 4;
    OSSL_PARAM *params = (OSSL_PARAM*)malloc(sizeof(OSSL_PARAM)*5);
    EVP_PKEY *pkey = nullptr;
    EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_from_name(nullptr, "DH", nullptr);

    if (!pctx) {
        return nullptr; // Handle error appropriately
    }

    EVP_PKEY_paramgen_init(pctx);

    params[0] = OSSL_PARAM_construct_uint("pbits", &pbits);
    params[1] = OSSL_PARAM_construct_uint("qbits", &qbits);
    params[2] = OSSL_PARAM_construct_utf8_string("type", (char*)"generator", 0);

    if (hash.empty()) {
        params[3] = OSSL_PARAM_construct_end();
    } else {
        params[3] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>(hash.data()), 0);
        params[4] = OSSL_PARAM_construct_end();
    }

    EVP_PKEY_CTX_set_params(pctx, params);
    EVP_PKEY_generate(pctx, &pkey);
    
    /* Free resources */
    EVP_PKEY_CTX_free(pctx);
    free(params);
    params = nullptr;
    pctx = nullptr;

    if (!pkey) {
        return nullptr;
    }

    return new DHparam(pkey);
}

} // namespace gssl::dhparam