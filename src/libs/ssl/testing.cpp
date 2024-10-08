#include <openssl/evp.h>
#include <openssl/obj_mac.h>
#include <openssl/pem.h>
#include <openssl/x509v3.h>
#include <openssl/x509.h>
#include <openssl/err.h>
#include "x509/x509.hpp"
#include <vector>

int main()
{
	char *buff = (char*)malloc(1024);
	RSAkeypair *key = gssl::rsa::generateKeypair();

	X509_REQ *csr = X509_REQ_new();
	X509_REQ_set_pubkey(csr, const_cast<EVP_PKEY*>(key->key()));
	std::vector<int> nids{NID_authority_key_identifier, NID_subject_key_identifier, NID_key_usage, NID_ext_key_usage, NID_basic_constraints};
	
	/* Set NIDS */
	X509_REQ_set_extension_nids(&nids[0]);
	auto name = X509_REQ_get_subject_name(csr);
	X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, (unsigned char*)"gurgui", -1, -1, 0);

	/* Set common_name */
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

	try
	{
		RSAkeypair *key = gssl::rsa::generateKeypair();
		if(key->key() == nullptr){
			return -1;
		}
	
		X509Cert _cert((unsigned char*)"gurgui", 10, key, 30);
		X509 *cert = _cert.getCertificate();
	
		if(cert == nullptr){
			fprintf(stderr, "couldn't create certificate\n");
			return -1;
		}

		//stack_st_X509_EXTENSION *ext_list = nullptr;
		// X509_EXTENSION *ext0 = X509V3_EXT_conf_nid(nullptr, nullptr, NID_authority_key_identifier, "keyid:always,issuer");
		// X509_EXTENSION *ext1 = X509V3_EXT_conf_nid(nullptr, nullptr, NID_subject_key_identifier, "hash");

		X509_EXTENSION *ext0 = X509V3_EXT_conf_nid(nullptr, nullptr, NID_authority_key_identifier, "bb:bb:bb:bb");
		X509_EXTENSION *ext1 = X509V3_EXT_conf_nid(nullptr, nullptr, NID_subject_key_identifier, "aa:aa:aa:aa");
		X509_EXTENSION *ext2 = X509V3_EXT_conf_nid(nullptr, nullptr, NID_key_usage, "cRLSign, keyCertSign");
		X509_EXTENSION *ext3 = X509V3_EXT_conf_nid(nullptr, nullptr, NID_ext_key_usage, "clientAuth");
		X509_EXTENSION *ext4 = X509V3_EXT_conf_nid(nullptr, nullptr, NID_basic_constraints, "CA:TRUE");
		
		ERR_error_string(ERR_get_error(), buff);
		fprintf(stderr, "%s\n", buff);
		
		//X509v3_add_ext(&ext_list, ext, -1);
		for(auto i : {ext0,ext3,ext2,ext4}){
			if(X509_add_ext(cert, i, -1) == 0){
				fprintf(stderr, "Adding extension failed\n");
				return -1;
			}	
		}

		if(_cert.sign(key) != 0){
			fprintf(stderr, "%s\n", _cert.getError().data());
			return -1;
		};	


		_cert.dump2("cert.pem");

	}catch(std::string &err){
		fprintf(stderr, "Caught exception -> %s\n", err.c_str());
	}		
	
	return 0;
}