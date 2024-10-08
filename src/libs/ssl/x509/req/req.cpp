/*
	char *buff = (char*)malloc(1024);
	RSAkeypair *key = gssl::rsa::generateKeypair();

	X509_REQ *csr = X509_REQ_new();
	X509_REQ_set_pubkey(csr, const_cast<EVP_PKEY*>(key->key()));
	std::vector<int> nids{NID_authority_key_identifier, NID_subject_key_identifier, NID_key_usage, NID_ext_key_usage, NID_basic_constraints};
	
	 Set NIDS 
	X509_REQ_set_extension_nids(&nids[0]);
	auto name = X509_REQ_get_subject_name(csr);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"gurgui", -1, -1, 0);

	/* Set common_name 
	X509_REQ_set_subject_name(csr, name);

	FILE *file = fopen("csr.pem", "wb");
	if(PEM_write_X509_REQ(file, csr) == 0){
		fprintf(stderr, "failed writing request\n");
	}else{
		fclose(file);
	};

	ERR_error_string(ERR_get_error(), buff);
	fprintf(stderr, "%s\n", buff);
	return 0;
*/
#include "req.hpp"
#include <openssl/x509.h>

X509Req::X509Req(ISmartPKEY *pkey):pkey(pkey),req(X509_REQ_new())
{
}

X509Req