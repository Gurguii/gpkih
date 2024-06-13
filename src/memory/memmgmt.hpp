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

// Buffer instance to manage dynamically allocated memory
// used by any part of the program that would require allocating dynamic memory (malloc())
// started by main() to 'MAYBE, NOT YET' allow modification of the buffer size from configuration file
// 'gpkih.conf'
extern Buffer *gpkihBuffer;

#define BUFFER_PTR gpkihBuffer;
#define BUFFER *gpkihBuffer
#define ALLOCATE gpkihBuffer->allocate
#define AVAILABLE_MEMORY gpkihBuffer->available()
#define CALLOCATE gpkihBuffer->allocate_and_copy
#define FREE_MEMORY_BLOCK gpkihBuffer->freeblock
#define BUFFER_DUMP gpkihBuffer->dump