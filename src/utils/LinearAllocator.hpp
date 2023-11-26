#pragma once

#include <cstdint>

typedef struct _LinearAllocator
    {
    uint8_t            *pool;
    uint32_t            capacity;
    uint32_t            head;
    uint32_t            allocations_cnt;
    } LinearAllocator;

    
void * LinearAllocator_Allocate( const uint32_t sz, LinearAllocator *allocator );
void * LinearAllocator_AllocateAligned( const uint32_t sz, const uint8_t alignment, LinearAllocator *allocator );
void   LinearAllocator_Destroy( LinearAllocator *allocator );
bool   LinearAllocator_Init( const uint32_t capacity, LinearAllocator *allocator );
void   LinearAllocator_Reset( LinearAllocator *allocator );