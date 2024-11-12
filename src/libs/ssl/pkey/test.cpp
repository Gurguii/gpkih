#include "dh.cpp"

int main()
{
	auto params = gssl::dhparam::generate("modp_2048", "sha512");
	
	const char *outpath = "./dhparam.pem";

	if(params == nullptr){
		fprintf(stderr, "Error: %s\n", params->lastError());
		return -1;
	}
	
	if(params->dump(outpath, "PEM") != 0){
		fprintf(stderr, "Error: %s\n", params->lastError());
		return 1;
	};

	printf("Succesfully dumped %s\n", outpath);
	
	params->dump("dhparam.pem", "PEM");
	params->dump("dhparam.der", "DER");

	return 0;
}