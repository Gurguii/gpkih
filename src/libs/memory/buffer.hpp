#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace gurgui::memory
{

class BufferException : public std::exception {
private:
  const char *msg;
public:
  BufferException() = default;
  BufferException(const char *msg);
  const char *what() const noexcept override; 
};

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
  size_t size();

  int freeblock(char *ptr, size_t *size = nullptr);
  size_t dump(const char *path, uint32_t blockSize = 4096);

  const std::string &getLastError();
  const char *const head();
};

}