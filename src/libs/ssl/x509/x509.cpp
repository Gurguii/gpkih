#include "x509.hpp"
#include <openssl/x509.h>

#undef UPDATE_ERROR
#define UPDATE_ERROR lastError.assign("");ERR_error_string(ERR_get_error(), &lastError[0])

X509Cert::X509Cert(unsigned char *cn, size_t serial, ISmartPKEY *key, size_t days):cn(cn){
	lastError.reserve(100);

	/* Initialize X509 structure */
	_cert = X509_new();
	if(_cert == nullptr){
		UPDATE_ERROR;
		throw(lastError);
	}

	/* Set common_name */
	X509_NAME *name;
	name = X509_get_subject_name(_cert);
	
	if(X509_NAME_add_entry_by_txt(name, "CN", MBSTRING_ASC, cn, -1, -1, 0) == 0){
		UPDATE_ERROR;
		throw(lastError);
	};

	if(X509_set_issuer_name(_cert, name) == 0){
		UPDATE_ERROR;
		throw(lastError);
	}
	
	/* Set serial */
	if(ASN1_INTEGER_set(X509_get_serialNumber(_cert), serial) == 0){
		UPDATE_ERROR;
		throw(lastError);
	};
	
	/* Set certificate validity */
	if(X509_gmtime_adj(X509_get_notBefore(_cert), 0) == nullptr || X509_gmtime_adj(X509_get_notAfter(_cert), days * 3600) == nullptr){
		UPDATE_ERROR;
		throw(lastError);
	}
	
	/* Set pubkey */
	if(X509_set_pubkey(_cert, const_cast<EVP_PKEY*>(key->key())) == 0){
		UPDATE_ERROR;
		throw(lastError);
	};
}

X509Cert::~X509Cert(){
	X509_free(_cert);
}

std::string &X509Cert::getError(){
	return lastError;
};

X509* const X509Cert::getCertificate(){
	return _cert;
}

int X509Cert::sign(ISmartPKEY* key, std::string_view hash){
	if(key->key() == nullptr){
		lastError = "Given key is NULL";
		return -1;
	}

	if(_hashMapping.find(hash.data()) == _hashMapping.end()){
		lastError = "Requested hash is not yet supported";
		return -1;
	}

	if(X509_sign(_cert, const_cast<EVP_PKEY*>(key->key()), _hashMapping[hash.data()]) == 0){
		UPDATE_ERROR;
		return -1;
	};

	return 0;
};

int X509Cert::dump(std::string_view filePath){
	FILE* file = fopen(filePath.data(), "wb");
	
	if(file == nullptr){
		lastError = "Couldn't open file";
		return -1;
	}

	return PEM_write_X509(file, _cert);
}

int X509Cert::dump(FILE *file){
	if(file == nullptr){
		lastError = "called X509Cert::dump() with a null pointer";
		return -1;
	}
	return PEM_write_X509(file, _cert);
}

int X509Cert::dump2(std::string_view filePath, std::string_view format)
{
	FILE *file = fopen(filePath.data(), "wb");
	
	if( file == nullptr){
		lastError = "Couldn't open file - ";
		lastError += filePath;
		return -1;
	}

	if(format == "pem"){
		PEM_write_X509(file,_cert);
	}else if(format == "der"){
		unsigned char *buff = nullptr;
		size_t size = i2d_X509(_cert, &buff);
		if(size <= 0){
			UPDATE_ERROR;
			return -1;
		}
		fwrite(buff, size, 1, file);
		free(buff);
	}else{	
		lastError = "UNKNOWN x509 format type ";
		lastError += format;
		return -1;
	}

	fclose(file);
	
	return 0;
}

size_t X509Cert::get_serial(){
	return serial;
}

unsigned char *X509Cert::get_cn(){
	return cn;
}