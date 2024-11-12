#include "dh.hpp"

#include <openssl/evp.h>
#include <openssl/params.h>
#include <openssl/encoder.h>

#include <vector>
#include <algorithm>

DHparam::DHparam(EVP_PKEY *pkey):_pkey(pkey){}

DHparam::~DHparam()
{
	printf("DHparam destructor...\n");
	fflush(stdout);
	EVP_PKEY_free(_pkey);
}

int DHparam::dump(std::string_view outPath, std::string_view outFormat)
{
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
    FILE *outfile = fopen(outPath.data(), "wb");

    if(outfile == nullptr){
    	_lastError = "Couldn't open file to write dhparams";
    	return 1;
    }

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

static std::vector<std::string_view>_valid_dh_implementations{"modp_2048", "modp_3072", "modp_4096", "modp_6144", "modp_8192", 
"ffdhe3072", "ffdhe4096", "ffdhe6144", "ffdhe8192"
};

static std::vector<std::string_view>_valid_msg_digest{"blake2b512","blake2s256", "md4","md5","mdc2","rmd160","sha1","sha224","sha256","sha3-224",
"sha3-256","sha3-384","sha3-512","sha384","sha512","sha512-224","sha512-256","shake128","shake256","sm3"
};

[[nodiscard("Returned structure, if not null, must be deleted manually with delete()")]]
DHparam* gssl::dhparam::generate(std::string_view dh_imp, const char *digest)
{
	if(std::find(_valid_dh_implementations.begin(), _valid_dh_implementations.end(), dh_imp) == _valid_dh_implementations.end()){
		return nullptr;
	};

	EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_DH, nullptr);
	OSSL_PARAM *params = (OSSL_PARAM*)malloc(sizeof(OSSL_PARAM)*3);
	EVP_PKEY *pkey = nullptr;

	if(pctx == nullptr || params == nullptr){
		return nullptr;
	}	

	params[0] = OSSL_PARAM_construct_utf8_string("group", (char*)dh_imp.data(), 0);
	//params[1] = OSSL_PARAM_construct_int("priv_len", &priv_len);
	
	if(digest == nullptr){
		params[1] = OSSL_PARAM_construct_end();	
	}else{
		if(std::find(_valid_msg_digest.begin(),_valid_msg_digest.end(), digest) == _valid_msg_digest.end()){
			EVP_PKEY_CTX_free(pctx);
			OSSL_PARAM_free(params);
			return nullptr;
		};
		params[1] = OSSL_PARAM_construct_utf8_string("digest", const_cast<char*>(digest), 0);	
		params[2] = OSSL_PARAM_construct_end();
	}

	EVP_PKEY_keygen_init(pctx);
	EVP_PKEY_CTX_set_params(pctx, params);
	EVP_PKEY_generate(pctx, &pkey);

	// At this point params were succesfully generated, context and params can be freed
	EVP_PKEY_CTX_free(pctx);
	free(params);

	auto param = new DHparam(pkey);

	return param;
}