#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <fmt/format.h>

/// @brief UNUSED custom exception
struct gpkihException : public std::exception{
  const char *errmsg;
  gpkihException(const char *msg):errmsg(msg){};
  const char *what(){
    return errmsg;
  }
};

enum class BUFFER_CREATION_RESULT{
  already_initialized,
  success,
  fail
};
constexpr const BUFFER_CREATION_RESULT BUFF_OK = BUFFER_CREATION_RESULT::success;
constexpr const BUFFER_CREATION_RESULT BUFF_FAIL = BUFFER_CREATION_RESULT::fail;
constexpr const BUFFER_CREATION_RESULT BUFF_ALREADY_INITIALIZED = BUFFER_CREATION_RESULT::already_initialized;

class Buffer{
private:
  std::vector<size_t> freed_sizes{};
  std::vector<char*> freed_ptrs{};

  char *memBlock = NULL;
  size_t memBlockSize = 0;

  char *next = NULL;
  size_t __availableBytes;
  bool __good = false;

  Buffer(size_t buffsize);
public:
  // TODO - develop this ( make ctor private )
  static BUFFER_CREATION_RESULT initialize(size_t buffSize, Buffer *&ptr);

  ~Buffer();

  char* allocate(size_t bytes);
  char* allocate_and_copy(char *&st, size_t *length, std::string_view src);
  
  bool good();
  size_t available();
  
  char *freeblock(char *ptr);
  size_t dump(const char *path, uint32_t block_size = 4096);
  size_t size();

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
#define FREEBLOCK gpkihBuffer->freeblock
#define BUFFER_DUMP gpkihBuffer->dump