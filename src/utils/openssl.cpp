#include "utils.hpp"

using namespace gpkih;

void utils::openssl::genkey(){
	return;
}

int utils::openssl::create_dhparam(std::string_view outpath, size_t size) {
  std::string command = fmt::format("openssl dhparam -quiet -out {} {}", outpath, size);
  if (system(command.c_str())) {
    return -1;
  }
  return 0;
}