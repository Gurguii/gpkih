#include "parser.hpp"

// SYNTAX : ./gpki build <profile> [subopts]
using namespace gpkih;
int parsers::build(std::vector<std::string> opts) {
  if (opts.empty()) {
    PERROR("profile must be given\n");
    PHINT("try 'gpki help build' for extra help\n");
    return 0;
  }

  strview profilename = opts[0];
  Profile *profile;

  if ((profile = db::profiles::load(profilename)) == nullptr) {
    PERROR("profile '{}' doesn't exist\n", profilename);
    return -1;
  }

  // std::cout << "Before loading configuration\n";
  // Subject _s{};
  // std::cout << _s.state << " " << _s.country << " " << _s.email << " "<<
  // _s.location << " "<< _s.organisation << "\n";
  //// Load profile configuration
  // ProfileConfig _conf(*profile);
  // Subject _defaults = _conf.default_subject(_conf);
  // std::cout << "After loading configuration\n";
  // std::cout << _defaults.state << " " << _defaults.country << " " <<
  // _defaults.email << " "<< _defaults.location << " "<< _defaults.organisation
  // << "\n";

  ProfileConfig configuration(*profile);
  if (configuration.succesfully_loaded == false) {
    seterror("couldn't load configuration files from profile '{}'",
             profile->name);
    return GPKIH_FAIL;
  }
  ConfigMap &pkiconf = *configuration.get(CONFIG_PKI);
  subopts::build params{
      .type = ET_NONE,
      .profile = profile,
  };
  params.config = &configuration;
  auto &type = params.type;

  opts.erase(opts.begin());
  opts.push_back("\0");

  // override default build params with user arguments and set
  for (int i = 0; i < opts.size(); ++i) {
    strview opt = opts[i];
    if (opt == "-ca" || opt == "--ca") {
      type = ENTITY_TYPE::ca;
    } else if (opt == "-cl" || opt == "--client") {
      type = ENTITY_TYPE::client;
    } else if (opt == "-sv" || opt == "--server") {
      type = ENTITY_TYPE::server;
    } else if (opt == "-keysize" || opt == "--keysize") {
      pkiconf["key"]["size"] = opts[++i];
    } else if (opt == "-keyformat") {
      pkiconf["key"]["creation_format"] = opts[++i];
    } else if (opt == "-outformat") {
      pkiconf["csr"]["creation_format"] = opts[++i];
    } else if (opt == "\0") {
      continue;
    } else {
      UNKNOWN_OPTION_MSG(opt);
    }
  }
  if (type == ENTITY_TYPE::none) {
    seterror("please specify an entity type -[ca|sv|cl]\n");
    return GPKIH_FAIL;
  }
  return actions::build(params);
}
