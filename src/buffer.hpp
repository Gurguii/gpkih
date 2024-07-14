#include "libs/memory/buffer.hpp"

class GpkihBuffer : public gurgui::memory::Buffer{
public:
  	const char* allocate_and_copy(const char *&st, size_t *length, std::string_view src);
  	explicit GpkihBuffer(size_t bytes);
};