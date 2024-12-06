#include "gssl.hpp"
#include <openssl/err.h>

void GSSL_ERR_UPDATE_PRINT(){
	ERR_error_string(ERR_get_error(), GSSL_ERR_BUFF);
	fprintf(stderr, "ERROR - %s\n", GSSL_ERR_BUFF);
}