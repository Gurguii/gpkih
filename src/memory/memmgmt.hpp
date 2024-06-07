#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>
#include <fmt/format.h>

struct gpkihException : public std::exception{
  const char *errmsg;
  gpkihException(const char *msg):errmsg(msg){};
  const char *what(){
    return errmsg;
  }
};

class Buffer{
private:
  std::vector<size_t> freed_sizes{};
  std::vector<char*> freed_ptrs{};

  char *memblock = NULL;
  char *next = NULL;
  size_t _available;
  size_t size; 

  bool _good = false;
public:
  Buffer(size_t buffsize);
  ~Buffer();
  char* allocate(size_t bytes);
  char* allocate_and_copy(char *&st, size_t *length, std::string_view src);
  bool good();
  size_t available();
  char *freeblock(char *ptr);
  
  // TESTING - not working
  size_t dump(const char *path, uint32_t block_size = 4096);
};

// Buffer instance to manage dynamically allocated memory
// used by any part of the program that would require allocating dynamic memory (malloc())
// started by main() to 'MAYBE, NOT YET' allow modification of the buffer size from configuration file
// 'gpkih.conf'
extern Buffer *__buff;

#define BUFFER_PTR __buff;
#define BUFFER *__buff
#define ALLOCATE __buff->allocate
#define AVAILABLE_MEMORY __buff->available()
#define CALLOCATE __buff->allocate_and_copy
#define FREEBLOCK __buff->freeblock