#pragma once
#include <cstdio>
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

class Buffer
{
private:
  std::vector<std::pair<size_t,std::byte*>>freedBlocks{};

  std::byte *memBlock = nullptr;
  const size_t memBlockSize = 0;
  
  std::byte *next = nullptr;
  size_t availableBytes;

  std::string lastError;
public:
  explicit Buffer(size_t size);
  ~Buffer();

  std::byte* allocate(size_t bytes);
  
  size_t available();
  size_t size();

  int freeblock(void *ptr, size_t size);

  size_t dump(const char *path, uint32_t blockSize = 4096);

  const std::string &getLastError();
  const std::byte *const head();
};

struct SmartMemBlock{
private:
  std::byte *__buffer = nullptr;
public:
  explicit SmartMemBlock(size_t bytes);
  std::byte *const get();
  ~SmartMemBlock();
};

}