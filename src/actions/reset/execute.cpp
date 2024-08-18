#include "AReset.hpp"

#include "../../libs/printing/printing.hpp"
#include "../../db/profiles.hpp"
#include "../../config/Config.hpp"
#include "../../gpkih.hpp"

using namespace gpkih;

int AReset::exec()
{
	DEBUG(1, "parsers::reset()");

	db::profiles::remove_all();
	bool autoans = Config::get("behaviour","autoanswer") == "yes" ? true : false;

	if(autoans == false){
		auto ans = PROMPT("About to reset gpkih, continue?", "[y/n]", true);		
		if(ans != "y" && ans != "yes"){
			return GPKIH_FAIL;			
		}
	}

	size_t deletedProfileFiles = 0;

	if(db::profiles::remove_all(&deletedProfileFiles) == GPKIH_FAIL){
		return GPKIH_FAIL;
	}

	size_t deletedSourceFiles = fs::remove_all(GPKIH_BASEDIR);
	if(deletedSourceFiles == 0){
		PERROR("Couldn't remove gpkih root directory '{}'\n", GPKIH_BASEDIR);
		return GPKIH_FAIL;
	};
 
	PSUCCESS("All gpkih files ({}) were deleted\n",deletedProfileFiles + deletedSourceFiles);
	return GPKIH_OK;
}