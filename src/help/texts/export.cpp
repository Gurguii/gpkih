namespace gpkih::help::exportdb
{
	const char *usage = R"(== export ==
export profile / entities data to any of the supported formats

[ syntax ]
  ./gpki export [subopts]

[ subopts ]
  -t | --type <sqlite|mysql|psql|json|csv>: mandatory, output format
  -o | --out <path> : mandatory, output path
  -p | --profile : optional, export specific profile (entities included) 
)";
	const char *examples = R"(== export examples ==
*Not implemented *
)";
}