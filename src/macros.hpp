#ifndef GPKIH_MACROS
#define GPKIH_MACROS

#pragma once

/* Logging */
#define ADD_LOG gpkihLogger->addLog

/* Buffer */
#define BUFFER_PTR gpkihBuffer;
#define BUFFER *gpkihBuffer
#define ALLOCATE gpkihBuffer->allocate
#define AVAILABLE_MEMORY gpkihBuffer->available()
#define CALLOCATE gpkihBuffer->allocate_and_copy
#define FREE_MEMORY_BLOCK gpkihBuffer->freeblock
#define BUFFER_DUMP gpkihBuffer->dump
#define ENOUGH_AVAILABLE(x) static_cast<bool>(gpkihBuffer->available()-x > 0)

#endif