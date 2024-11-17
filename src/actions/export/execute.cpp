#include "AExport.hpp"

#include "../../libs/printing/printing.hpp"
#include "../../consts.hpp"

#include <filesystem>
#include <unordered_map>

#include "sqlite/export.hpp"
#include "csv/export.hpp"
#include "mysql/export.hpp"
#include "postgres/export.hpp"

static std::unordered_map<std::string, int(*)(std::string_view, std::vector<std::string> &args)> exportFunctions{
	{"csv", gpkih::csv::exportDB},
	
	#ifdef GPKIH_PSQL
	{"psql", gpkih::postgres::exportDB},
	#endif

	#ifdef GPKIH_SQLITE
	{"sqlite", gpkih::sqlite::exportDB},
	#endif

	#ifdef GPKIH_MYSQL
	{"mysql", gpkih::mysql::exportDB},
	#endif
};

// ./gpkih export -t | --type <sqlite|json|csv|psql|mysql>  -o | --out <outDir> ... <subopts>

int AExport::exec(std::vector<std::string> &args) const {
	DEBUG(1, "AExport::exec()");

	/* BEG - Parse arguments */
	if(args.empty()){
		PINFO("Missing mandatory -type and --out options, try gpkih help export\n");
		return GPKIH_OK;
	}

	std::string_view outDir = ".";
	auto exportFunction = exportFunctions.end();

	for(int i = 0; i < args.size(); ++i){
		std::string_view opt = args[i];
		if(opt == "-o" || opt == "-out" || opt == "--out"){
			outDir = args[++i];
			if(std::filesystem::is_directory(outDir) == false){
				PERROR("Given path '{}' is not a directory\n", outDir);
				return GPKIH_FAIL;
			}
			args.erase(args.begin()+i);
		}else if(opt == "-t" || opt == "-type" || opt == "--type"){
			exportFunction = exportFunctions.find(args[++i]);			
			if(exportFunction == exportFunctions.end()){
				PERROR("Type doesn't exist, valid types - ");
				for(const auto &[typeName, typeFunc] : exportFunctions){
					fmt::print(S_ERROR, "{} ", typeName);
				}
				std::cout << "\n";
				return GPKIH_FAIL;
			};
		}
	}
	/* END - Parse arguments */

	/* BEG - Check arguments */
	if(exportFunction == exportFunctions.end()){
		PINFO("No type given, try gpkih help export\n");
		return GPKIH_FAIL;
	}
	/* END - Check arguments */

	/* BEG - Call export function */
	return exportFunction->second(outDir, args);
	/* END - Call export function */
}