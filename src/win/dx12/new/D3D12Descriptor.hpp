#pragma once
#include <stdint.h>

#include "D3D12Types.hpp"
#include "Utilities.hpp"

namespace D3D12 { namespace Descriptor
{

static const bool IS_SHADER_ACCESS = true;
static const bool IS_NO_SHADER_ACCESS = false;

typedef struct _LocalHandle
    {
    D3D12_CPU_DESCRIPTOR_HANDLE
                        hcpu;
    } LocalHandle;

typedef struct _FreeHeap
    {
    Core::IMDescriptorHeap
                       *heap;
    D3D12_CPU_DESCRIPTOR_HANDLE
                        cpu_start;
    D3D12_DESCRIPTOR_HEAP_TYPE
                        heap_type;
    uint16_t            descriptor_size;
    uint32_t            capacity;
    uint32_t            size;
    uint32_t           *frees;
    } FreeHeap;

typedef struct _MappedHandle
    {
    D3D12_CPU_DESCRIPTOR_HANDLE
                        hcpu;
    D3D12_GPU_DESCRIPTOR_HANDLE
                        hgpu;
    } MappedHandle;

typedef struct _RingHeapFrame
    {
    uint32_t            tail;
    uint32_t            count;
    } RingHeapFrame;

typedef struct _RingHeap
    {
    Core::IMDescriptorHeap
                       *heap;
    D3D12_CPU_DESCRIPTOR_HANDLE
                        cpu_start;
    D3D12_GPU_DESCRIPTOR_HANDLE
                        gpu_start;
    D3D12_DESCRIPTOR_HEAP_TYPE
                        heap_type;
    uint32_t            capacity;
    uint32_t            tail;
    RingHeapFrame       frames[ 2 ];
    uint8_t             current_frame_index;
    uint16_t            descriptor_size;
    } RingHeap;


LocalHandle FreeHeap_Allocate( FreeHeap *heap );
bool        FreeHeap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE heap_type, const uint32_t capacity, FreeHeap *heap );
void        FreeHeap_Destroy( FreeHeap *heap );
void        FreeHeap_Free( LocalHandle *to_free, FreeHeap *heap );

MappedHandle RingHeap_Allocate( RingHeap *heap );
bool         RingHeap_AllocateArray( const uint32_t count, MappedHandle *out_arr, RingHeap *heap );
bool         RingHeap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE heap_type, const uint32_t capacity, RingHeap *heap );
void         RingHeap_Destroy( RingHeap *heap );
uint32_t     RingHeap_GetSize( const RingHeap *heap );
void         RingHeap_NextFrame( RingHeap *heap );


/*******************************************************************
*
*   CPUHandleToIndex()
*
*   DESCRIPTION:
*       Given a descriptor handle, retrieve its index within the
*       heap from which it was allocated.
*
*******************************************************************/

static inline uint32_t CPUHandleToIndex( const D3D12_CPU_DESCRIPTOR_HANDLE handle, const D3D12_CPU_DESCRIPTOR_HANDLE start, const uint16_t sz )
{
debug_assert( ( handle.ptr - start.ptr ) % sz == 0 );
uint32_t ret = (uint32_t)( ( handle.ptr - start.ptr ) / sz );

return( ret );

} /* CPUHandleToIndex() */


/*******************************************************************
*
*   GPUHandleToIndex()
*
*   DESCRIPTION:
*       Given a descriptor handle, retrieve its index within the
*       heap from which it was allocated.
*
*******************************************************************/

static inline uint32_t GPUHandleToIndex( const D3D12_GPU_DESCRIPTOR_HANDLE handle, const D3D12_GPU_DESCRIPTOR_HANDLE start, const uint16_t sz )
{
debug_assert( ( handle.ptr - start.ptr ) % sz == 0 );
uint32_t ret = (uint32_t)( ( handle.ptr - start.ptr ) / sz );

return( ret );

} /* CPUHandleToIndex() */


/*******************************************************************
*
*   OffsetCPUHandle()
*
*   DESCRIPTION:
*       Obtain a CPU descriptor handle.
*
*******************************************************************/

static inline D3D12_CPU_DESCRIPTOR_HANDLE OffsetCPUHandle( const D3D12_CPU_DESCRIPTOR_HANDLE start, const uint32_t index, const uint16_t sz )
{
D3D12_CPU_DESCRIPTOR_HANDLE ret = start;
ret.ptr += ( sz * index );

return( ret );

} /* OffsetCPUHandle() */


/*******************************************************************
*
*   OffsetGPUHandle()
*
*   DESCRIPTION:
*       Obtain a GPU descriptor handle.
*
*******************************************************************/

static inline D3D12_GPU_DESCRIPTOR_HANDLE OffsetGPUHandle( const D3D12_GPU_DESCRIPTOR_HANDLE start, const uint32_t index, const uint16_t sz )
{
D3D12_GPU_DESCRIPTOR_HANDLE ret = start;
ret.ptr += ( sz * index );

return( ret );

} /* OffsetGPUHandle() */


} } /* namespace D3D12::Descriptor */