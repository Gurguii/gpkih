#include "actions.hpp"
using namespace gpki;
static inline str styled_key_value_separator = " = ";
static inline str styled_title(const char *st) {
  return fmt::format(fg(WHITE) | EMPHASIS::bold, st);
}
static inline str styled_key_prop(strview key, strview val,
                                  ENTITY_TYPE t = ET_NONE) {
  return fmt::format(fg(t == ET_SV ? GREEN : (t == ET_CL ? CYAN : BLACK)) |
                         EMPHASIS::bold,
                     "{}", key) +
         styled_key_value_separator +
         fmt::format(fg(WHITE) | EMPHASIS::bold | EMPHASIS::italic, "{}", val);
}

int actions::get(subopts::get &params) {
  Profile &profile = params.profile;
  if (!params.cl_properties.empty()) {
    for (auto &prop : params.cl_properties) {
      if (GpkihConfig::client.find(prop.data()) != GpkihConfig::client.end()) {
        PRINTF("{}.{}\n", styled_title("client"),
               styled_key_prop(prop, GpkihConfig::client[prop.data()], ET_CL));
      }    
    }
  }
  if (!params.sv_properties.empty()) {
    for (auto &prop : params.sv_properties) {
      if (GpkihConfig::server.find(prop.data()) != GpkihConfig::server.end()) {
        PRINTF("{}.{}\n", styled_title("server"),
               styled_key_prop(prop, GpkihConfig::server[prop.data()], ET_SV));
      }
    }
  }
  if (!params.common_properties.empty()) {
    for (auto &prop : params.common_properties) {
      PRINTF("{}.{}\n", styled_title("common"),
             styled_key_prop(prop, GpkihConfig::common[prop.data()], ET_SV));
    }
  }
  // ./gpki get <profile> prop1,prop2...propN
  // ./gpki get test client.
  return 0;
}
int actions::set(subopts::set &params){
    // ./gpki set <profile> property1=value1 property2=value2 .... propertyN=valueN
    // ./gpki set test client.remote='142.250.200.78 12345'
    return 0;
}
