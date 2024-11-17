#include "req.hpp"

#include <openssl/crypto.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include <openssl/pem.h>
#include <openssl/encoder.h>

#include "../../gssl.hpp"

[[nodiscard("Returned pointer must be manually freed with free()")]]
static char* get_hex_SKI(ISmartPKEY *pkey){

	/* BEG - encode public key in DER format */
	auto *enc_ctx = OSSL_ENCODER_CTX_new_for_pkey(pkey->key(), EVP_PKEY_PUBLIC_KEY, "DER", nullptr, nullptr);
    unsigned char* pdata = nullptr;
    size_t pdata_len = 0;
    OSSL_ENCODER_to_data(enc_ctx, &pdata, &pdata_len);
    OSSL_ENCODER_CTX_free(enc_ctx);
    /* END - encode public key in DER format */

    /* BEG - generate public key's hash */
    unsigned char ski[SHA256_DIGEST_LENGTH];
    unsigned int ski_len = sizeof(ski);

	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
	EVP_DigestUpdate(mdctx, pdata, pdata_len);
	EVP_DigestFinal_ex(mdctx, ski, &ski_len);
	EVP_MD_CTX_free(mdctx);
	/* END - generate public key's hash */
	
	/* BEG - convert SKI to hex */
	char *hex_ski = (char*)malloc(ski_len*2+1);
	for (int i = 0; i < ski_len; i++) {
	    sprintf(hex_ski + (i * 2), "%02x", ski[i]);
	}
	hex_ski[ski_len * 2] = '\0';
	/* END - convert SKI to hex */

	return hex_ski;
}

static int add_server_ext(){
	return 0;
}

static int add_ca_ext(){
	return 0;
}

X509Req::X509Req(ISmartPKEY *pkey, std::string_view entityType, std::string_view cn, 
	std::string_view country, 
	std::string_view location, 
	std::string_view organisation, 
	std::string_view state
):_pkey(pkey),_req(X509_REQ_new(), X509_REQ_free)
{
	if(!X509_REQ_set_pubkey(_req.get(), const_cast<EVP_PKEY*>(pkey->key()))){
		ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
		throw(GSSL_ERR_BUFF);
	}

	auto SKI = get_hex_SKI(_pkey); // Subject key identifier (public key digest in hex format)

	X509V3_CTX *ctx = nullptr;
	X509V3_set_ctx(ctx, nullptr, nullptr, _req.get(), nullptr, 0);
	STACK_OF(X509_EXTENSION) *exts = sk_X509_EXTENSION_new_null();

	/* BEG - add appropiate X509 extensions */
	if(entityType == "client"){
		X509_EXTENSION *bc_ext 	 = X509V3_EXT_conf(nullptr, ctx, "basicConstraints", "CA:FALSE");
    	X509_EXTENSION *ku_ext 	 = X509V3_EXT_conf(nullptr, ctx, "keyUsage", "digitalSignature,keyEncipherment,keyAgreement");
    	X509_EXTENSION *key_ext  = X509V3_EXT_conf(nullptr, ctx, "extendedKeyUsage", "clientAuth");
    	X509_EXTENSION *subj_ext = X509V3_EXT_conf(nullptr, ctx, "subjectKeyIdentifier", SKI);
    	for(auto ext : {bc_ext, ku_ext, key_ext, subj_ext}){
    		if(ext){
    			sk_X509_EXTENSION_push(exts, ext);	
    		}else{
	
    			fprintf(stderr, "Failed adding extension to stack\n");
        		ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
				fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
    		}
    	}
	}else if(entityType == "server"){
		X509_EXTENSION *bc_ext 	 = X509V3_EXT_conf(nullptr, ctx, "basicConstraints", "CA:FALSE");
		X509_EXTENSION *ku_ext 	 = X509V3_EXT_conf(nullptr, ctx, "keyUsage", "digitalSignature,keyEncipherment,nonRepudiation");
		X509_EXTENSION *key_ext  = X509V3_EXT_conf(nullptr, ctx, "extendedKeyUsage", "serverAuth");
		X509_EXTENSION *subj_ext = X509V3_EXT_conf(nullptr, ctx, "subjectKeyIdentifier", SKI);
		for(auto ext : {bc_ext, ku_ext, key_ext, subj_ext}){
    		if(ext){
    			sk_X509_EXTENSION_push(exts, ext);	
    		}else{
	
    			fprintf(stderr, "Failed adding extension to stack\n");
        		ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
				fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
    		}
    	}
	}else if(entityType == "ca"){
		X509_EXTENSION *bc_ext 	 = X509V3_EXT_conf(nullptr, ctx, "basicConstraints", "critical,CA:TRUE");
		X509_EXTENSION *ku_ext 	 = X509V3_EXT_conf(nullptr, ctx, "keyUsage", "cRLSign, keyCertSign");
		X509_EXTENSION *subj_ext = X509V3_EXT_conf(nullptr, ctx, "subjectKeyIdentifier", SKI);
		// authorityKeyIdentifier=keyid:always,issuer
		for(auto ext : {bc_ext, ku_ext, subj_ext}){
    		if(ext){
    			sk_X509_EXTENSION_push(exts, ext);	
    		}else{
    			fprintf(stderr, "Failed adding extension to stack\n");
        		ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
				fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
    		}
    	}
	}

	if (!X509_REQ_add_extensions(_req.get(), exts)) {
        fprintf(stderr, "Failed adding extensions' stack to certificate request\n");
        ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
		fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
    }

    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
	/* END - add appropiate X509 extensions */

    /* BEG - set subject */
	auto name = X509_NAME_new();
	if(!X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)cn.data(), -1, -1, 0)){
		fprintf(stderr, "Error setting common name\n");
		ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
				fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
	}

	if(!country.empty()){
		if(country.size() == 2){
			if(!X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)country.data(), -1, -1, 0)){
				fprintf(stderr, "Error setting country\n");
				ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
				fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
			}
		}
	}

	if(!location.empty()){
		if(!X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (unsigned char*)location.data(), -1, -1, 0)){
			fprintf(stderr, "Error setting country\n");
			ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
			fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
		}
	}

	if(!organisation.empty()){
		if(!X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)organisation.data(), -1, -1, 0)){
			fprintf(stderr, "Error setting country\n");
			ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
			fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
		}
	}

	if(!state.empty()){
		if(!X509_NAME_add_entry_by_txt(name, "ST", MBSTRING_ASC, (unsigned char*)state.data(), -1, -1, 0)){
			fprintf(stderr, "Error setting country\n");
			ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
			fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
		}
	}
	/* END - set subject */

	/* BEG - sign request */
	if(!X509_REQ_sign(_req.get(), _pkey->key(), EVP_sha512())){
		fprintf(stderr, "Error signing request\n");
		ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
			fprintf(stderr, "%s\n", GSSL_ERR_BUFF);
	}
	/* END - sign request */
}

FILE* X509Req::dump(FILE *outFile){
	if(!PEM_write_X509_REQ(outFile, _req.get())){
		fclose(outFile);
		return nullptr;
	};
	return outFile;
}

int X509Req::dump(std::string_view outFile){
	FILE* file = fopen(outFile.data(), "wb");
	if(file == nullptr){
		memset(GSSL_ERR_BUFF, 0, sizeof(GSSL_ERR_BUFF));
		memcpy(GSSL_ERR_BUFF, "Couldn't open file", 19);
		return -1;
	}

	if(!PEM_write_X509_REQ(file, _req.get())){
		memset(GSSL_ERR_BUFF, 0, sizeof(GSSL_ERR_BUFF));
		memcpy(GSSL_ERR_BUFF, "Couldn't dump file", 19);
		return -1;
	}
	
	fclose(file);

	return 0;
}