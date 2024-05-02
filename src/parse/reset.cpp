#include "parser.hpp"
#include "../db/profiles.hpp"

int gpkih::parsers::reset(std::vector<std::string> &opts){
	db::profiles::remove_all();
	fs::remove_all(GPKIH_BASEDIR);

	return GPKIH_OK;
}