#include "generate.hpp"
#include "../../consts.hpp"
#include "../../libs/printing/printing.hpp"
#include "../../libs/ssl/pkey/dh.hpp"
#include <string>

// ./gpkih dhparam generate --size 2048 --hash sha256 --outformat PEM --out dhparam2048.pem
int generate(std::vector<std::string> &args){
	/* BEG - default params */
	const char *hash = "sha256";
	unsigned int pbits = 2048;

	std::string_view outformat = "PEM";
	FILE *out= stdout;
	/* END - default params */
	
	/* BEG - parse args */
	for(int i = 0; i < args.size(); ++i){
		std::string_view arg = args[i];
		if(arg == "-h" || arg == "--hash"){
			hash = args[++i].data();
		}else if(arg == "-s" || arg == "--size"){
			pbits = std::stoul(args[++i]);
		}else if(arg == "-o" || arg == "--out"){
			out = fopen(args[++i].data(), "wb");
			if(out == nullptr){
				PERROR("Couldn't open output file {}\n", args[i]);
				return GPKIH_FAIL;
			}
		}else if(arg == "-of" || arg == "--outformat"){
			outformat = args[++i];
		}
	}
	/* END - parse args */

	/* BEG - generate params */
	fprintf(stderr, "-- Generating DH params...\n");
	gssl::dhparam::DHparam* dhparams = gssl::dhparam::generate(pbits, hash);

	if(dhparams == nullptr){
		PERROR("Couldn't generate dhparams\n");
		fclose(out);
		return GPKIH_FAIL;
	}

	fprintf(stderr, "-- Diffie-Hellman parameters of %u bits and %s hash generated\n", pbits, hash); 
	/* END - generate params */

	/* BEG - write params to file */
	if(dhparams->dump(out, outformat) != GPKIH_OK){
		PERROR(dhparams->lastError().c_str());
	};
	/* END - write params to file */
	
	/* BEG - resource cleanup */ 
	delete(dhparams);
	fclose(out);
	/* END - resource cleanup */

	return GPKIH_OK;
}