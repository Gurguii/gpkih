#include "dhparam.hpp"

constexpr const char *_usage = R"(
Action: dhparam
Description: Do stuff with dhparams

== SYNTAX ==
  ./gpki dhparam <command> [subopts]
  
== COMMANDS ==
g | gen | generate : generate dh params

== COMMANDS' SUBOPTS ==
  
  * generate *
  -g    | --group : specify desired group, default modp_2048
  -hash | --hash   : specify hash algorithm, default sha256
  -o | --out   : file to output the generated params, default stdout
  -of | --outformat : desired output format, default PEM
)";

constexpr const char *_examples = R"(== dhparam examples ==
  1. Generate dhparam with default values and save it to file
    ./gpkih dhparam generate > dhparam2048.pem

  2. Generate dhparam using a different group of parameters and hash
    ./gpkih dhparam generate --group ffdhe3072 --hash sha512

  3. Generate dhparam, save it to file with DER format
    ./gpkih dhparam generate --outformat DER --out dhparam2048.pem 
)";

const char *ADHparam::usage() const {
	return _usage;
};

const char *ADHparam::examples() const {
	return _examples;
}