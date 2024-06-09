#include <cstring>
#include <cstddef>
#include <fstream>

#include "memmgmt.hpp"

Buffer *gpkihBuffer = NULL; // this will be initialized by main()

/* BEG - class Buffer */
BUFFER_CREATION_RESULT Buffer::initialize(size_t buffSize, Buffer *&ptr){
	static Buffer *uniqueBuffer = NULL;
	
	if(uniqueBuffer == NULL){
		uniqueBuffer = new Buffer(buffSize);
		if(uniqueBuffer->__good == false){
			return BUFF_FAIL;
		}
		ptr = uniqueBuffer;
		return BUFF_OK;
	}

	ptr = uniqueBuffer;
	return BUFF_ALREADY_INITIALIZED;
};

Buffer::Buffer(size_t buffsize):memBlockSize(buffsize){
	if(memBlockSize <= 0){
		return;
	}

	memBlock = (char*)malloc(buffsize);
	
	if(memBlock == NULL){
	  	return;
	}

	memset(memBlock,0,buffsize);
	next = memBlock;

	__availableBytes = buffsize;
	__good = true;
};

Buffer::~Buffer(){
	free(memBlock);
	memBlock = NULL;
}

char *Buffer::allocate(size_t bytes){
    if(__availableBytes <= bytes){
    	// PROTOTYPE - HAVE NEVER TESTED THIS
    	// Look for available freed_blocks where data can fit
    	for(int i = 0; i < freed_sizes.size(); ++i){
    		if(freed_sizes[i] >= bytes+1){
    			// It fits here
    			size_t diff = freed_sizes[i] - bytes+1; 
    			if(diff == 0){
    				// takes the whole chunk
    				char *ptr = freed_ptrs[i];
    				freed_ptrs.erase(freed_ptrs.begin() + i);
    				freed_sizes.erase(freed_sizes.begin() + i);
    				return ptr;
    			}else if (diff > 1){
    				// There will be some free space
    				// note that im ignoring that single byte difference on purpose cause 
    				// the Buffer class always add '\0' at the end of the allocated
    				// data so that subsequent allocated data doesn't get printed 
    				// unintentionally on a call to printf() or any trouble with functions
    				// that rely on the closing null byte
    				char *ptr = freed_ptrs[i];
    				freed_ptrs.erase(freed_ptrs.begin() +i);
    				freed_sizes.erase(freed_sizes.begin() +i);
    				// add a new entry with the 'leftovers'
    				freed_ptrs.emplace_back(ptr+bytes+2);
    				freed_sizes.emplace_back(diff);
    			}
    		}	
    	}

    	// Reallocate to fit requested data + 4096 bytes
    	size_t newSize = Buffer::memBlockSize + bytes + 4096;
    	auto newMemBlock = (char*)realloc(Buffer::memBlock, newSize);
	    if(newMemBlock == nullptr){
			return nullptr;	
	    }
	    Buffer::memBlock = newMemBlock;
	    Buffer::memBlockSize = newSize;
	    next=Buffer::memBlock+Buffer::memBlockSize-4096-bytes;
	}
	   
	char *ptr = next;
	next+=bytes;
	*next='\0';
	++next;
	__availableBytes -= bytes;

	return ptr;
};

char *Buffer::allocate_and_copy(char *&st, size_t *length, std::string_view src){

	if(length == nullptr){
		// Don't set/use length 
		size_t _len = src.size();
		st = allocate(_len);
		memcpy(st, src.data(), _len);
	}else{
		// Set/use length
		*length = src.size();
		st = allocate(*length);
		memcpy(st, src.data(), *length);	
	}

	return st;
}

bool Buffer::good(){
	return __good;
}

size_t Buffer::available(){
	return __availableBytes;
}

char *Buffer::freeblock(char *ptr){
	if(ptr < memBlock || ptr > (memBlock + memBlockSize)){
		// Out of memBlock, not allocated by this instance
		return NULL;
	};

	size_t length = strlen(ptr);
	memset(ptr, 0, length);
	
	freed_sizes.emplace_back(length);
	freed_ptrs.emplace_back(ptr);

	return ptr;
	//return freed_blocks.try_emplace(length, ptr).second ? ptr : NULL;
}

/* END - class Buffer */

size_t Buffer::dump(const char *path, uint32_t block_size){
	std::ofstream file(path,std::ios::binary);

	if(!file.is_open()){
		return -1;
	}

	char *current = memBlock;
	size_t iters = memBlockSize / block_size;
	size_t rest = memBlockSize % block_size;

	size_t written = 0;

	for(int i = 0; i < iters; ++i){
		file.write(current, block_size);
		written += block_size;
		if(file.fail()){
			fprintf(stderr, "Wrote %lu bytes before failing\n", written);
		}
		current+=block_size;
	}

	file.write(current,rest);

	if(file.fail()){
		fprintf(stderr, "Failed after writing rest\n");
		return -1;
	}

	file.close();
	return 0;
}

size_t Buffer::size(){
	return memBlockSize;
}

const char *const Buffer::head(){
	return memBlock;
}