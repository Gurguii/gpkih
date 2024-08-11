#include "../parser.hpp"
#include "../../experimental/export.hpp"
#include <unordered_map>

using namespace gpkih;

// SYNTAX: ./gpkih export -<sqlite|mysql|psql|csv|json> -profile <profile> -o|out <filepath>
/*
	[ opts ]
	-o | --out <path> : mandatory, output path
	-t | --type <sqlite|mysql|psql|csv|json> : mandatory, output format
*/

std::unordered_map<std::string, int(*)(std::string_view)> exportFunctions{
	{"sqlite",gpkih::db::exportSQLITE},
	{"mysql",gpkih::db::exportMYSQL},
	{"psql",gpkih::db::exportPSQL},
	{"json", gpkih::db::exportJSON},
	{"csv", gpkih::db::exportCSV}
};

int parsers::dbexport(std::vector<std::string> &opts){
	DEBUG(1, "parsers::dbexport()");

	if(opts.empty()){
		PINFO("Missing mandatory -type and --out options, try gpkih help export\n");
		return GPKIH_OK;
	}

	std::string_view outFile;
	std::unordered_map<std::string, int(*)(std::string_view)>::iterator exportFunction = exportFunctions.end();

	for(int i = 0; i < opts.size(); ++i){
		std::string_view opt = opts[i];
		if(opt == "-o" || opt == "--out"){
			outFile = opts[++i];
		}else if(opt == "-t" || opt == "--type"){
			exportFunction = exportFunctions.find(opts[++i]);
			if(exportFunction == exportFunctions.end()){
				PERROR("Type {} doesn't exist, valid types - sqlite | mysql | psql | json | csv\n", opts[i]);
				return GPKIH_FAIL;
			};
		}
	}

	if(outFile.empty()){
		PINFO("No outpath given, try gpkih help export\n");
		return GPKIH_FAIL;
	}

	if(exportFunction == exportFunctions.end()){
		PINFO("No type given, try gpkih help export\n");
		return GPKIH_FAIL;
	}
	
	return exportFunction->second(outFile);
}