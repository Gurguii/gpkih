#include "logger.hpp"
#include <exception>
#include <filesystem>
#include <fstream>
#include <unordered_map>

using namespace gpkih;
Logger::Logger(std::string filepath):file(filepath){
	if(!std::filesystem::exists(file)){
		// Try to create the file
		std::ofstream f(file);
		if(!f.is_open()){
			throw std::exception();
		}
		// file created
	}
}

void Logger::add(std::string_view msg, Level level){
	
}

int Logger::cleanup_with_exit() {
  fmt::print("\n");
  std::vector<std::future<int>> tasks{};
  // launch every task asynchronously
  for (auto cleanup_func : cleanup_functions) {
    tasks.push_back(std::move(std::async(std::launch::async, cleanup_func)));
  }
  PROGRAMEXITING();
  // wait for every task to finish
  for (auto &t : tasks) {
    t.wait();
  }
  exit(0);
}