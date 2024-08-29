#pragma once

#include <cstdint>

static const void *LINEAR_ALLOCATOR_DETACHED_BASE_ADDRESS = (void*)0x1000000000000000;

typedef struct _LinearAllocator
    {
    uint8_t            *pool;
    uint64_t            capacity;
    uint64_t            head;
    uint32_t            allocations_cnt;
    bool                is_own_pool;
    } LinearAllocator;

typedef struct _LinearAllocatorResetToken
    {
#if defined( _DEBUG )
    uint8_t            *pool;
#endif
    uint32_t            allocations_cnt;
    uint64_t            head;
    } LinearAllocatorResetToken;

    
void *                    LinearAllocator_Allocate( const uint64_t sz, LinearAllocator *allocator );
void *                    LinearAllocator_AllocateAligned( const uint64_t sz, const uint64_t alignment, LinearAllocator *allocator );
void                      LinearAllocator_Destroy( LinearAllocator *allocator );
void                      LinearAllocator_InitDetached( const uint64_t capacity, LinearAllocator *allocator );
LinearAllocatorResetToken LinearAllocator_GetResetToken( LinearAllocator *allocator );
bool                      LinearAllocator_Init( const uint64_t capacity, LinearAllocator *allocator );
bool                      LinearAllocator_InitAttached( const uint64_t capacity, void *buffer, LinearAllocator *allocator );
void                      LinearAllocator_Reset( LinearAllocator *allocator );
void                      LinearAllocator_ResetByToken( const LinearAllocatorResetToken token, LinearAllocator *allocator );