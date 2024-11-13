#include "dhparam.hpp"
#include "generate.hpp"
#include "../../consts.hpp"
#include "../../libs/ssl/pkey/dh.hpp"

int ADHparam::exec(std::vector<std::string> &args) const {

	/* BEG - parse arguments */
	for(auto &arg : args)
	{
		if(arg == "g" || arg == "gen" || arg == "generate"){
			return generate(args);
		}
	}
	/* END - parse arguments */

	return GPKIH_OK;
}