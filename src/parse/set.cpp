#include "parser.hpp"

using namespace gpkih;

int parsers::set(std::vector<str> opts)
{
	// Check profile
	if (opts.empty()) {
		PERROR("profile must be given\n");
		return GPKIH_FAIL;
	}
	return GPKIH_OK;
}