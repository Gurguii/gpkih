#include <cstring>
#include "memmgmt.hpp"
#include "printing/printing.hpp"
#include "utils/utils.hpp"

Buffer *__buff = NULL; // this will be initialized by main()

/* BEG - class Buffer */

Buffer::Buffer(size_t buffsize){
	memblock = (char*)malloc(buffsize);
	
	if(memblock == NULL){
	  PERROR("couldn't allocate requested memory");
	  return;
	}

	memset(memblock,0,buffsize);
	next = memblock;

	_available = buffsize;
};

Buffer::~Buffer(){
	free(memblock);
	memblock = NULL;
}

char *Buffer::allocate(size_t bytes){

    if(_available <= bytes){
    	// PROTOTYPE - HAVE NEVER TESTED THIS
    	// Look for available freed_blocks where data can fit
    	for(int i = 0; i < freed_sizes.size(); ++i){
    		if(freed_sizes[i] >= bytes+1){
    			// It fits here
    			size_t diff = freed_sizes[i] - bytes+1; 
    			if( diff == 0){
    				// takes the whole chunk
    				char *ptr = freed_ptrs[i];
    				freed_ptrs.erase(freed_ptrs.begin() + i);
    				freed_sizes.erase(freed_sizes.begin() + i);
    				return ptr;
    			}else if (diff > 1){
    				// there will be some free space
    				// note im ignoring that single byte difference on purpose cause 
    				// the Buffer class always add '\0' at the end of the allocated
    				// data so that subsequent allocated data doesn't get printed 
    				// unintentionally on a call to printf()
    				char *ptr = freed_ptrs[i];
    				freed_ptrs.erase(freed_ptrs.begin() +i);
    				freed_sizes.erase(freed_sizes.begin() +i);
    				// add a new entry with the 'leftovers'
    				freed_ptrs.emplace_back(ptr+bytes+2);
    				freed_sizes.emplace_back(diff);
    			}
    		}	
    	}
	    PERROR("not enough memory left to allocate, '{}' bytes available, '{}' bytes requested", _available, bytes);
	    return NULL;
	}
	   
	char *ptr = next; 
	next+=bytes;
	*next='\0';
	++next;
	_available -= bytes;
	
	PDEBUG(3,"allocated '{}' bytes, '{}' available", bytes, _available);

	return ptr;
};

char *Buffer::allocate_and_copy(char * &st, size_t *length, std::string_view src){
	if(length == nullptr){
		// Don't set/use length 
		size_t _len = src.size() + 1;
		st = allocate(_len);
		memcpy(st, src.data(), _len);
	}else{
		// Set/use length
		*length = src.size() +1;
		st = allocate(*length);
		memcpy(st, src.data(), *length);	
	}

	return st;
}

size_t Buffer::available(){
	return _available;
}

char *Buffer::freeblock(char *&ptr){
	if(ptr < memblock || ptr > (memblock + size)){
		// Out of memblock, not allocated by this instance
		return NULL;
	};
	size_t length = slen(ptr);
	memset(ptr, 0, length);
	
	PDEBUG(1,"memblock: {:p} - freed memory: {:p} - size freed: {} - inmediate next: {:p}",memblock,ptr, length, ptr+length);
	
	freed_sizes.emplace_back(length);
	freed_ptrs.emplace_back(ptr);

	return ptr;
	//return freed_blocks.try_emplace(length, ptr).second ? ptr : NULL;
}
/* END - class Buffer */

// TESTING STUFF
size_t Buffer::dump(const char *path, uint32_t block_size){
	std::ofstream file(path,std::ios::binary);
	if(!file.is_open()){
		PERROR("couldn't open file '{}' for dumping buffer\n", path);
		return -1;
	}

	char *current = memblock;
	size_t iters = size / block_size;
	size_t rest = size % block_size;

	for(int i = 0; i < iters; ++i){
		file.write(current, block_size);
		current+=block_size;
	}

	file.write(current,rest);

	if(file.fail()){
		PERROR("error dumping buffer '{}'\n",path);
		return -1;
	}

	file.close();
	PINFO("dumped {} bytes to {}\n", size, path);
	return 0;
}