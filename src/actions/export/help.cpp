#include "AExport.hpp"

constexpr const char *_usage = R"(== export ==
export profile / entities data to any of the supported formats

[ syntax ]
  ./gpki export [subopts]

[ subopts ]
  -t | --type <sqlite|mysql|psql|json|csv>: mandatory, output format
  -o | --out <path> : mandatory, output path
  !UNIMPLEMENTED -p | --profile : optional, export specific profile (entities included) 
)";

constexpr const char *_examples = R"(== export examples ==
  1. Export the whole database.
    ./gpkih export -t sqlite -o ./gpkih_sqlite.db
)";

const char *AExport::usage() const{
	return _usage;
};

const char *AExport::examples() const{
	return _examples;
}