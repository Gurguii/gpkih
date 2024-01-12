#include <fstream>
#include <iostream>
#include <sstream>
#include <string_view>
#include <unordered_map>
#ifndef EOL
#ifdef __WIN32
#define EOL "\r\n"
#else
#define EOL "\n"
#endif
#endif
int sed(std::string_view src, std::string_view dst,
        std::unordered_map<std::string_view, std::string_view> vals) {
  std::ifstream srcfile(src.data());
  if (!srcfile.is_open()) {
    return -1;
  }
  std::ofstream dstfile(dst.data());
  if (!dstfile.is_open()) {
    return -1;
  }
  std::string line;
  std::string word;
  while (getline(srcfile, line)) {
    auto ss = std::stringstream(line);
    while (ss >> word) {
      if (vals.find(word) != vals.end()) {
        dstfile << vals[word];
      } else {
        dstfile << word;
      }
      dstfile << " ";
    }
#ifdef __WIN32
    dstfile << "\n";
  }
#else
    dstfile << "\r\n";
  }
#endif
  srcfile.close();
  dstfile.close();
  return 0;
}
int main() {
  if (sed("/home/gurgui/base", "/home/gurgui/aftersed.txt",
          {{"GPKI_BASEDIR", "WISKONSIN"}})) {
    std::cout << "error\n";
    return 0;
  } else {
    std::cout << "success\n";
    return 1;
  }
}
