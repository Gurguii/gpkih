#include "parser.hpp"

using namespace gpkih;

int parsers::set(std::vector<std::string> &opts)
{
	// Check profile
	if (opts.empty()) {
		PERROR("profile must be given\n");
		return GPKIH_FAIL;
	}
	return GPKIH_OK;
}