#include "dhparam.hpp"

constexpr const char *_usage = R"(Action: dhparam
Description: Do stuff with dhparams

== SYNTAX ==
  ./gpki dhparam <command> [subopts]
  
== COMMANDS ==
g | gen | generate : generate dh params

== GENERATE SUBOPTS ==
-s    | --size      : key size in bits, default 2048
-h    | --hash      : specify hash algorithm, default sha256
-o    | --out       : file to output the generated params, default stdout
-of   | --outformat : desired output format, default PEM
)";

constexpr const char *_examples = R"(== EXAMPLES ==
  1. Generate dhparam with default values and save it to file
    ./gpkih dhparam generate > dhparam2048.pem

  2. Generate dhparam of 4096 bits using sha512 as digest
    ./gpkih dhparam generate --size 4096 --hash sha512

  3. Generate dhparam, save it to file with DER format
    ./gpkih dhparam generate --outformat DER --out dhparam2048.der 
)";

const char *ADHparam::usage() const {
	return _usage;
};

const char *ADHparam::examples() const {
	return _examples;
}