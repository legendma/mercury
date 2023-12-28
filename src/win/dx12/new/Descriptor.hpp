#pragma once
#include "RenderCore.hpp"

namespace Render { namespace Descriptor
{

typedef struct _Handle
    {
    D3D12_CPU_DESCRIPTOR_HANDLE
                        cpu_hndl;
    D3D12_GPU_DESCRIPTOR_HANDLE
                        gpu_hndl;
    } Handle;

static const bool HEAP_IS_LINEAR = false;
static const bool HEAP_IS_RINGBUFFER = true;
typedef struct _Heap
    {
    D3D12_CPU_DESCRIPTOR_HANDLE
                        cpu_start;
    D3D12_GPU_DESCRIPTOR_HANDLE
                        gpu_start;
    uint32_t            descriptor_size;
    uint32_t            capacity;
    D3D12_DESCRIPTOR_HEAP_TYPE
                        type;
    uint8_t             frames_since_wrap;
    bool                is_shader_visible;
    bool                is_ring;
    uint32_t            head;
    ID3D12DescriptorHeap
                       *heap;
    } Heap;
    

Handle Heap_Allocate( Heap *heap );
bool   Heap_AllocateBatch( const uint32_t allocate_cnt, uint32_t out_capacity, Handle *out, Heap *heap );
bool   Heap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool is_ring, const uint32_t capacity, Core::IMDevice *device, Heap *heap );
void   Heap_Destroy( const bool should_defer, Heap *heap );
void   Heap_NextFrame( Heap *heap );
void   Heap_Reset( Heap *heap );

} } /* namespace Render::Descriptor */