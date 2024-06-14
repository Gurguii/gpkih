#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <fmt/format.h>

class Buffer{
private:
  std::vector<std::pair<size_t,char*>>freedBlocks{};

  char *memBlock = nullptr;
  const size_t memBlockSize = 0;
  
  char *next = nullptr;
  size_t availableBytes;

  std::string lastError;
public:
  explicit Buffer(size_t size);
  ~Buffer();

  char* allocate(size_t bytes);
  char* allocate_and_copy(char *&st, size_t *length, std::string_view src);
  
  size_t available();
  
  char *freeblock(char *ptr, size_t *size = nullptr);
  size_t dump(const char *path, uint32_t blockSize = 4096);
  size_t size();

  const std::string &getLastError();
  const char *const head();
};