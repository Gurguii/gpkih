#include "parser.hpp"
#include <utility>
#include <variant>

using namespace gpki;
using SCOPE = ENTITY_TYPE;

int parsers::get(std::vector<str> opts) {
  // ./gpki <profile> prop1=val1 prop2=val2 ... propN=valN
  // ./gpki set test client.server='142.250.201.78 9422'
  if (opts.empty()) {
    PERROR("profile name must be given\n");
    PHINT("try gpki help get\n");
    return -1;
  }

  strview profilename = opts[0];
  subopts::get params{};
  Profile &profile = params.profile;

  if (db::profiles::load(profilename, profile)) {
    PERROR("profile '{}' doesn't exist\n", profilename);
    return -1;
  }

  opts.erase(opts.begin());
  GpkihConfig::set(profile);

  str section, prop; // valid scopes -> common client server
  sstream ss;
  std::vector<str> nonexist_props;

  for (str &properties : opts) {
    ss.clear();
    ss << properties;
    getline(ss, section, '.');
    if (section == "client" || section == "cl") {
      getline(ss, prop);
      if (prop.empty() && params.cl_properties.empty()) {
        for (const auto &kv : GpkihConfig::client) {
          params.cl_properties.emplace_back(kv.first);
        }
        continue;
      }
      if (GpkihConfig::client_prop_exists(prop)) {
        params.cl_properties.emplace_back(prop);
        continue;
      }
      nonexist_props.emplace_back(prop);
    } else if (section == "server" || section == "sv") {
      getline(ss, prop);
      if (prop.empty() && params.sv_properties.empty()) {
        for (const auto &kv : GpkihConfig::server) {
          params.sv_properties.emplace_back(kv.first);
        }
        continue;
      }
      if (GpkihConfig::server_prop_exists(prop)) {
        params.sv_properties.emplace_back(prop);
        continue;
      }
      nonexist_props.emplace_back(prop);
    } else if (section == "common" || section == "com") {
      getline(ss, prop);
      if (prop.empty() && params.common_properties.empty()) {
        for (const auto &kv : GpkihConfig::server) {
          params.common_properties.emplace_back(kv.first);
        }
        continue;
      }
      if (GpkihConfig::common_prop_exists(prop)) {
        params.common_properties.emplace_back(prop);
        continue;
      }
      nonexist_props.emplace_back(prop);
    }
  }

  if (!nonexist_props.empty()) {
    PWARN("unexistant properties: ");
    for (auto &property : nonexist_props) {
      PRINTF(fg(COLOR::medium_violet_red) | EMPHASIS::italic, "{} ", property);
    }
    std::cout << "\n";
  }

  return actions::get(params);
}
