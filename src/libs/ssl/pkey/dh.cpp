#include "dh.hpp"

#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/encoder.h>

#include <algorithm>
#include <array>

DHparam::DHparam(EVP_PKEY *pkey):_pkey(pkey){}

DHparam::~DHparam()
{
	EVP_PKEY_free(_pkey);
}

int DHparam::dump(FILE *outfile, std::string_view outFormat)
{
	if(outfile == nullptr){
		return -1;
	}

	if(outFormat != "PEM" && outFormat != "DER" && outFormat != "TEXT"){
		_lastError = "Invalid output format not among: PEM,DER,TEXT";
		return -1;
	}

	OSSL_ENCODER_CTX *ctx = OSSL_ENCODER_CTX_new_for_pkey(_pkey, EVP_PKEY_KEY_PARAMETERS, outFormat.data(), nullptr, nullptr);
    
    if (!ctx) {
    	_lastError = "Failed initializing context";
        return 1;
    }

    /* BEG - dump dhparams to file */
    if(OSSL_ENCODER_to_fp(ctx, outfile) == 0){
        _lastError = "Failed writing to file";
        return 1;
    };

    fclose(outfile);

    OSSL_ENCODER_CTX_free(ctx);
    /* END - dump dhparams to file */

	return 0;
}

const char *DHparam::lastError(){
	return _lastError;
}

[[nodiscard("Returned structure, if not null, must be freed manually with delete()")]]
DHparam* gssl::dhparam::generate(unsigned int pbits, std::string_view digest)
{

	//int gindex = 2;
	unsigned int qbits = pbits/4;
	OSSL_PARAM *params = (OSSL_PARAM*)malloc(sizeof(OSSL_PARAM)*5);
	EVP_PKEY *pkey = NULL;
	EVP_PKEY_CTX *pctx = NULL;
	pctx = EVP_PKEY_CTX_new_from_name(NULL, "DH", NULL);
	
	EVP_PKEY_paramgen_init(pctx);
	
	params[0] = OSSL_PARAM_construct_uint("pbits", &pbits);
	params[1] = OSSL_PARAM_construct_uint("qbits", &qbits);
	params[2] = OSSL_PARAM_construct_utf8_string("type", (char*)"generator", 0);
	
	if(digest.empty()){
		params[3] = OSSL_PARAM_construct_end();	
	}else{
		params[3] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>(digest.data()), 0);
		params[4] = OSSL_PARAM_construct_end();
	}
	
	EVP_PKEY_CTX_set_params(pctx, params);
	
	EVP_PKEY_generate(pctx, &pkey);
	
	//BIO *obio = BIO_new_fd(0, 0);
	//EVP_PKEY_print_params(outb, pkey, 0, NULL);
	//EVP_PKEY_print_private(outb, key, 0, NULL);

	// At this point params were succesfully generated, context and params can be freed
	EVP_PKEY_CTX_free(pctx);
	free(params);

	auto param = new DHparam(pkey);

	return param;
}