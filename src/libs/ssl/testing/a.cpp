#include <openssl/err.h>
#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/pem.h>
#include <openssl/sha.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/encoder.h>
#include "../x509/x509.hpp"

int main()
{
	char *buff = (char*)malloc(1024);
	RSAkeypair *key = gssl::rsa::generateKeypair();

	X509_REQ *csr = X509_REQ_new();

	if(!X509_REQ_set_pubkey(csr, const_cast<EVP_PKEY*>(key->key()))){
		fprintf(stderr, "Failed setting pubkey\n");
		return -1;
	};

	/* BEG - encode public key in DER format */
    auto *enc_ctx = OSSL_ENCODER_CTX_new_for_pkey(key->key(), EVP_PKEY_PUBLIC_KEY, "DER", nullptr, nullptr);
    
    unsigned char* pdata = nullptr;
    size_t pdatalen = 0;
    OSSL_ENCODER_to_data(enc_ctx, &pdata, &pdatalen);
    /* END - encode public key in DER format */

    /* BEG - generate public key's hash */
    unsigned char ski[SHA256_DIGEST_LENGTH];
	unsigned int ski_len = sizeof(ski);
	
	EVP_MD_CTX *mdctx = EVP_MD_CTX_new();
	EVP_DigestInit_ex(mdctx, EVP_sha256(), nullptr);
	EVP_DigestUpdate(mdctx, pdata, pdatalen);
	EVP_DigestFinal_ex(mdctx, ski, &ski_len);
	EVP_MD_CTX_free(mdctx);
	/* END - generate public key's hash */

	/* BEG - convert to hex (SKI) */
	char hex_ski[ski_len * 2 + 1]; // Allocate enough space for hex string
	for (int i = 0; i < ski_len; i++) {
	    sprintf(hex_ski + (i * 2), "%02x", ski[i]); // Correctly format each byte
	}
	hex_ski[ski_len * 2] = '\0'; // Null-terminate the string
	/* END - convert to hex (SKI) */

	//char hex_ski[ski_len*3];
	//for(int i = 0; i < ski_len; i+=3){
	//	sprintf(hex_ski+i, "%02x:", i);
	//}
	printf("HEX_SKI -> %s\n", hex_ski);

    /* BEG - add x509v3 requested extensions */
    X509V3_CTX *ctx = nullptr;
    X509V3_set_ctx(ctx, nullptr, nullptr, csr, nullptr,0);
    STACK_OF(X509_EXTENSION) *exts = sk_X509_EXTENSION_new_null();

    const char *san_value = "DNS:gurgui.com,DNS:www.gurgui.com";
    X509_EXTENSION *bc_ext 	 = X509V3_EXT_conf(nullptr, ctx, "basicConstraints", "CA:FALSE");
    X509_EXTENSION *ku_ext 	 = X509V3_EXT_conf(nullptr, ctx, "keyUsage", "digitalSignature,keyEncipherment,keyAgreement");
    X509_EXTENSION *key_ext  = X509V3_EXT_conf(nullptr, ctx, "extendedKeyUsage", "clientAuth");
    X509_EXTENSION *subj_ext = X509V3_EXT_conf(nullptr, ctx, "subjectKeyIdentifier", hex_ski);

    //X509_EXTENSION *san_ext  = X509V3_EXT_conf(nullptr, ctx, "subjectAltName", (char *)san_value);
    //X509_EXTENSION *auth_ext = X509V3_EXT_conf(nullptr, ctx, "authorityKeyIdentifier", "DD:CC:BB:AA,AB:BC:DC:FF");

    for(X509_EXTENSION* ext : {bc_ext, ku_ext, key_ext, subj_ext}){
    	if(ext){
    		sk_X509_EXTENSION_push(exts, ext);	
    	}else{

    		fprintf(stderr, "Failed adding extension to stack\n");
        	ERR_error_string(ERR_get_error(), buff);
			fprintf(stderr, "%s\n", buff);
    	}
    }

    if (!X509_REQ_add_extensions(csr, exts)) {
        fprintf(stderr, "Failed to add extensions to CSR\n");
        ERR_error_string(ERR_get_error(), buff);
		fprintf(stderr, "%s\n", buff);
        return -1;
    }

    sk_X509_EXTENSION_pop_free(exts, X509_EXTENSION_free);
    /* END - add x509v3 requested extensions */

	/* BEG - create and set subject */
	auto name = X509_NAME_new();

	if( name == nullptr ){
		fprintf(stderr, "name == NULL\n");
		return -1;
	}
	
	if(!X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"gurgui", -1, -1, 0)){
		fprintf(stderr, "Setting common name\n");
		return -1;
	}

	if(!X509_NAME_add_entry_by_txt(name, "C", MBSTRING_ASC, (unsigned char*)"ES", -1, -1, 0)){
		fprintf(stderr, "Setting country\n");
		return -1;
	}

	if(!X509_NAME_add_entry_by_txt(name, "L", MBSTRING_ASC, (unsigned char*)"ELEPE", -1, -1, 0)){
		fprintf(stderr, "Setting location\n");
		return -1;
	}

	if(!X509_NAME_add_entry_by_txt(name, "O", MBSTRING_ASC, (unsigned char*)"PORRO", -1, -1, 0)){
		fprintf(stderr, "Setting organisation\n");
		return -1;
	}

	if(!X509_NAME_add_entry_by_txt(name, "ST", MBSTRING_ASC, (unsigned char*)"ESPANITA", -1, -1, 0)){
		fprintf(stderr, "Setting state\n");
		return -1;
	}

	if(!X509_REQ_set_subject_name(csr, name)){
		fprintf(stderr, "Setting subject\n");
		return -1;
	};
	/* END - create and set subject */

	/* BEG - sign the request */
	if(!X509_REQ_sign(csr, const_cast<EVP_PKEY*>(key->key()), EVP_sha512())){
		fprintf(stderr, "Failed signing request\n");
		return -1;
	};
	/* END - sign the request */
	
	/* BEG - write request to file */
	FILE *file = fopen("csr.pem", "wb");

	if(!PEM_write_X509_REQ(file, csr)){
		fprintf(stderr, "failed writing request\n");
	}

	fclose(file);
	/* END - write request to file */
	return 0;
}