#include "buffer.hpp"
#include <cstring>
#include <fstream>

using namespace gurgui::memory;

BufferException::BufferException(const char *msg):msg(msg){}
const char *BufferException::what() const noexcept{
	return msg;
}

/* BEG - class Buffer */
Buffer::Buffer(size_t size):memBlockSize(size),availableBytes(size){

	if(memBlockSize <= 0){
		throw BufferException("Memory block's size has to be a positive integer");
		return;
	}

	memBlock = (char*)calloc(size, sizeof(uint8_t));
	
	if(memBlock == nullptr){
		throw BufferException("Couldn't allocate memory, calloc() returned nullptr");
	  	return;
	}

	next = memBlock;
};

Buffer::~Buffer(){
	free(memBlock);
	memBlock = nullptr;
}

char *Buffer::allocate(size_t bytes){
	//if(bytes == 0){
	//	return nullptr;
	//}
    if(availableBytes <= bytes){
    	// PROTOTYPE - HAVE NEVER TESTED THIS
    	// Look for available freed blocks where data can fit
    	for(int i = 0; i < freedBlocks.size(); ++i){
    		const auto &[size,address] = freedBlocks[i]; 
    		if(size >= bytes+1){
    			// It fits here
    			size_t diff = size - bytes+1;
    			if(diff == 0){
    				// takes the whole chunk
    				char *ptr = address;
    				freedBlocks.erase(freedBlocks.begin() + i);
    				return ptr;
    			}else if (diff > 1){
    				// There will be some free space
    				// note that im ignoring that single byte difference on purpose cause 
    				// the Buffer class always add '\0' at the end of the allocated
    				// data so that subsequent allocated data doesn't get printed 
    				// unintentionally on a call to printf() or any trouble with functions
    				// that rely on the closing null byte
    				char *ptr = address;
    				// add a new entry with the 'leftovers'
    				freedBlocks.erase(freedBlocks.begin() + i);
    				freedBlocks.emplace_back(diff, ptr + bytes + 2);
    				return ptr;
    			}
    		}	
    	}

    	// TODO - FIX THIS
    	freedBlocks.emplace_back(availableBytes, next);

    	auto newMemBlock = (char*)calloc(memBlockSize,sizeof(uint8_t));
	    
	    if(newMemBlock == nullptr){
	    	lastError = "Couldn't allocate a new memory block";
	    }

	    memBlock = newMemBlock;
	    availableBytes = memBlockSize;
	    next = memBlock;
	}
	
	char *ptr = next;
	next+=bytes;
	*next='\0';
	++next;
	availableBytes -= bytes;

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

size_t Buffer::available(){
	return availableBytes;
}

int Buffer::freeblock(char *ptr, size_t *size){
	if(ptr < memBlock || ptr > (memBlock + memBlockSize)){
		lastError = "Cannot free a pointer outside the managed memory block";
		return -1;
	}

	// Set freed memory block to 0 to avoid leaks
	// The performance difference might be huge between calls depending
	// on if size is given, cause if not the length of the ptr will get
	// calculated, and if its a big ass array it might be a pain
	size_t length = size == nullptr ? strlen(ptr): *size;
	
	memset(ptr, 0, length);
	availableBytes+=length+1; // length + null byte

	// Keep track of freed blocks
	freedBlocks.emplace_back(length,ptr);

	return 0;
}

size_t Buffer::dump(const char *path, uint32_t blockSize){
	std::ofstream file(path,std::ios::binary);

	if(!file.is_open()){
		return -1;
	}

	char *current = memBlock;
	size_t iters = memBlockSize / blockSize;
	size_t rest = memBlockSize % blockSize;

	size_t written = 0;

	for(int i = 0; i < iters; ++i){
		file.write(current, blockSize);
		written += blockSize;
		if(file.fail()){
			lastError = "Something failed while dumping";
			return -1;
		}
		current+=blockSize;
	}

	file.write(current,rest);

	if(file.fail()){
		lastError = "Failed dumping last few bytes";
		return -1;
	}

	file.close();
	return 0;
}

size_t Buffer::size(){
	return memBlockSize;
}

const std::string& Buffer::getLastError(){
	return lastError;
}

const char *const Buffer::head(){
	return memBlock;
}