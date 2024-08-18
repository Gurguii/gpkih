#include "AExport.hpp"

#include "../../libs/printing/printing.hpp"
#include "../../consts.hpp"

#include <unordered_map>

#include "sqlite/export.hpp"

static std::unordered_map<std::string, int(*)(std::string_view)> exportFunctions{
	{"sqlite",gpkih::sqlite::exportDB},
};

int AExport::exec(){
	DEBUG(1, "AExport::exec()");

	/* BEG - Parse arguments */
	if(args.empty()){
		PINFO("Missing mandatory -type and --out options, try gpkih help export\n");
		return GPKIH_OK;
	}

	std::string_view outFile;
	std::unordered_map<std::string, int(*)(std::string_view)>::iterator exportFunction = exportFunctions.end();

	for(int i = 0; i < args.size(); ++i){
		std::string_view opt = args[i];
		if(opt == "-o" || opt == "--out"){
			outFile = args[++i];
		}else if(opt == "-t" || opt == "--type"){
			exportFunction = exportFunctions.find(args[++i]);
			if(exportFunction == exportFunctions.end()){
				PERROR("Type {} doesn't exist, valid types - sqlite | mysql | psql | json | csv\n", args[i]);
				return GPKIH_FAIL;
			};
		}
	}
	/* END - Parse arguments */

	/* BEG - Check arguments */
	if(outFile.empty()){
		PINFO("No outpath given, try gpkih help export\n");
		return GPKIH_FAIL;
	}

	if(exportFunction == exportFunctions.end()){
		PINFO("No type given, try gpkih help export\n");
		return GPKIH_FAIL;
	}
	/* END - Check arguments */

	/* BEG - Call export function */
	return exportFunction->second(outFile);
	/* END - Call export function */
}