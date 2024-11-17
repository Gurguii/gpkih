#include "ifaces.hpp"
#include <openssl/bio.h>
#include <openssl/pem.h>

ISmartPKEY::ISmartPKEY(EVP_PKEY *key):_key(key,EVP_PKEY_free){};

ISmartPKEY::~ISmartPKEY(){
	if(_key != nullptr){
		EVP_PKEY_free(_key.get());
	}
}

EVP_PKEY *ISmartPKEY::key(){
	return _key.get();
}

FILE *ISmartPKEY::pem_dump_private(FILE *file){
	if(file == nullptr){
		return nullptr;
	}
	if(PEM_write_PrivateKey(file, _key.get(), nullptr, nullptr, 0, nullptr, nullptr) <= 0){
		return nullptr;
	}
	return file;
}

int ISmartPKEY::pem_dump_private(std::string_view filePath){
	FILE* file = fopen(filePath.data(), "wb");
	if(file == nullptr){
		return -1;
	}
	if(PEM_write_PrivateKey(file, _key.get(), nullptr, nullptr, 0, nullptr, nullptr)){
		return -1;
	}
	fclose(file);
	return 0;
}

FILE *ISmartPKEY::pem_dump_public(FILE *file){
	if(file == nullptr){
		return nullptr;
	}
	if(PEM_write_PUBKEY(file, _key.get()) <= 0){
		return nullptr;
	}
	return file;
}

int ISmartPKEY::pem_dump_public(std::string_view filePath){
	FILE* file = fopen(filePath.data(), "wb");
	if(file == nullptr){
		return -1;
	}
	if(PEM_write_PUBKEY(file, _key.get()) <= 0){
		return -1;
	}
	fclose(file);
	return 0;
}