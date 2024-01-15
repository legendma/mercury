#include <stdint.h>
#include <stdlib.h>

#include "D3D12Core.hpp"
#include "D3D12Descriptor.hpp"
#include "D3D12Initializers.hpp"
#include "Utilities.hpp"

namespace D3D12 { namespace Descriptor
{

compiler_assert( cnt_of_array( ((RingHeap*)NULL)->frames ) == Core::FRAME_COUNT, D3D12Descriptor_cpp );


/*******************************************************************
*
*   GetRingHeapHead()
*
*   DESCRIPTION:
*       Get the current head of the given ring heap.
*
*******************************************************************/

static inline uint32_t GetRingHeapHead( const RingHeap *heap )
{
uint64_t head = heap->tail + RingHeap_GetSize( heap );
head %= heap->capacity;

return( (uint32_t)head );

} /* GetRingHeapHead() */


/*******************************************************************
*
*   GetRingHeapHeadGivenSize()
*
*   DESCRIPTION:
*       Get the current head of the given ring heap, given that
*       heap's current size..
*
*******************************************************************/

static inline uint32_t GetRingHeapHeadGivenSize( const uint32_t size, const RingHeap *heap )
{
uint64_t head = heap->tail + size;
head %= heap->capacity;

return( (uint32_t)head );

} /* GetRingHeapHeadGivenSize() */


/*******************************************************************
*
*   IsShaderAccessAllowed()
*
*   DESCRIPTION:
*       Is the given descriptor heap type allowed to be created with
*       shader visible descriptors?
*
*******************************************************************/

static inline bool IsShaderAccessAllowed( const D3D12_DESCRIPTOR_HEAP_TYPE heap_type )
{
return( heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
     || heap_type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );

} /* IsShaderAccessAllowed() */


static void ReleaseFreeHeap( FreeHeap *heap );
static void ReleaseRingHeap( RingHeap *heap );


/*******************************************************************
*
*   FreeHeap_Allocate()
*
*   DESCRIPTION:
*       Allocate a single handle from the free list heap.
*
*******************************************************************/

LocalHandle FreeHeap_Allocate( FreeHeap *heap )
{
LocalHandle ret = {};
if( heap->capacity == heap->size )
    {
    debug_assert_always();
    return( ret );
    }

uint32_t index = heap->frees[ heap->capacity - ( heap->size + 1 ) ];
ret.hcpu = OffsetCPUHandle( heap->cpu_start, index, heap->descriptor_size );

heap->size++;

return( ret );

} /* FreeHeap_Allocate() */


/*******************************************************************
*
*   FreeHeap_Create()
*
*   DESCRIPTION:
*       Create a new free-list allocated descriptor heap.  The
*       descriptors managed by this module are cpu-only, and must be
*       copied to shader visible registers for use on the GPU.
*
*******************************************************************/

bool FreeHeap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE heap_type, const uint32_t capacity, FreeHeap *heap )
{
clr_struct( heap );

Core::IMDevice *device = Core::GetDevice();

D3D12_DESCRIPTOR_HEAP_DESC desc = Initializers::GetDescriptorHeapDescriptor( capacity, TRUE, heap_type );
if( capacity == 0
 || FAILED( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &heap->heap ) ) ) )
    {
    debug_assert_always();
    ReleaseFreeHeap( heap );
    return( false );
    }

heap->frees = (uint32_t*)malloc( capacity * sizeof(*heap->frees) );
if( !heap->frees )
    {
    debug_assert_always();
    ReleaseFreeHeap( heap );
    return( false );
    }

for( uint32_t i = 0; i < capacity; i++ )
    {
    heap->frees[ capacity - ( i + 1 ) ] = i;
    }

heap->cpu_start       = heap->heap->GetCPUDescriptorHandleForHeapStart();
heap->heap_type       = heap_type;
heap->descriptor_size = device->GetDescriptorHandleIncrementSize( heap_type );
heap->capacity        = capacity;

return( true );

} /* FreeHeap_Create() */


/*******************************************************************
*
*   FreeHeap_Destroy()
*
*   DESCRIPTION:
*       Release the free list descriptor heap's resources.
*
*******************************************************************/

void FreeHeap_Destroy( FreeHeap *heap )
{
ReleaseFreeHeap( heap );

} /* FreeHeap_Destroy() */


/*******************************************************************
*
*   FreeHeap_Free()
*
*   DESCRIPTION:
*       Relinquish the given descriptor handle, and return it to the
*       free pool for reuse.
*
*******************************************************************/

void FreeHeap_Free( LocalHandle *to_free, FreeHeap *heap )
{
if( !to_free )
    {
    return;
    }

debug_assert( to_free->hcpu.ptr != 0 );

uint32_t index = CPUHandleToIndex( to_free->hcpu, heap->cpu_start, heap->descriptor_size );
debug_assert( index < heap->capacity );
debug_if( TRUE,
    uint32_t free_count = heap->capacity - heap->size;
    for( uint32_t i = 0; i < free_count; i++ )
        {
        debug_assert( heap->frees[ i ] != index );
        }
    );

heap->frees[ heap->capacity - heap->size ] = index;
heap->size--;

clr_struct( to_free );

} /* FreeHeap_Free() */


/*******************************************************************
*
*   RingHeap_Allocate()
*
*   DESCRIPTION:
*       Allocate a single handle from the ring buffer heap.
*
*******************************************************************/

MappedHandle RingHeap_Allocate( RingHeap *heap )
{
uint32_t current_size = RingHeap_GetSize( heap );
MappedHandle ret = {};
if( heap->capacity == current_size )
    {
    debug_assert_always();
    return( ret );
    }

uint32_t head = GetRingHeapHeadGivenSize( current_size, heap );
RingHeapFrame *frame = &heap->frames[ heap->current_frame_index ];

ret.hcpu = OffsetCPUHandle( heap->cpu_start, head, heap->descriptor_size );
ret.hgpu = OffsetGPUHandle( heap->gpu_start, head, heap->descriptor_size );

frame->count++;

return( ret );

} /* RingHeap_Allocate() */


/*******************************************************************
*
*   RingHeap_AllocateArray()
*
*   DESCRIPTION:
*       Allocate a contiguous array of handles from the ring buffer
*       heap.
*
*******************************************************************/

bool RingHeap_AllocateArray( const uint32_t count, MappedHandle *out_arr, RingHeap *heap )
{
uint32_t current_size = RingHeap_GetSize( heap );
if( heap->capacity - current_size < count )
    {
    debug_assert_always();
    return( false );
    }

uint32_t head = GetRingHeapHeadGivenSize( current_size, heap );
RingHeapFrame *frame = &heap->frames[ heap->current_frame_index ];
if( head > heap->tail )
    {
    uint32_t remain_till_wrap = heap->capacity - head;
    if( remain_till_wrap < count )
        {
        if( heap->capacity - ( current_size + remain_till_wrap ) < count )
            {
            debug_assert_always();
            return( false );
            }

        frame->count += remain_till_wrap;
        head = 0;
        }
    }

for( uint32_t i = 0; i < count; i++ )
    {
    MappedHandle *out = &out_arr[ i ];
    out->hcpu = OffsetCPUHandle( heap->cpu_start, head + i, heap->descriptor_size );
    out->hgpu = OffsetGPUHandle( heap->gpu_start, head + i, heap->descriptor_size );

    frame->count++;
    }

return( true );

} /* RingHeap_AllocateArray() */


/*******************************************************************
*
*   RingHeap_Create()
*
*   DESCRIPTION:
*       Create a new ring allocated descriptor heap.  The
*       descriptors managed by this module are mapped from CPU to
*       GPU, allowing use in shaders.  The allocator manages the
*       lifetime of descriptors via 'frames'.  Frames are activated
*       sequentially, i.e. [Frame0][Frame1][Frame2][Frame0]... and
*       repeat once the max number of frames have been reached.
*       When a new frame begins, all descriptors allocated for the
*       frame the last time it occurred, are automatically freed.
*       This mechanic allows descriptors to be reused over and
*       again - while guaranteeing that a descriptor is not modified
*       whilst the GPU could still be using it.
*
*******************************************************************/

bool RingHeap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE heap_type, const uint32_t capacity, RingHeap *heap )
{
clr_struct( heap );
if( !IsShaderAccessAllowed( heap_type ) )
    {
    hard_assert_always();
    return( false );
    }

Core::IMDevice *device = Core::GetDevice();

D3D12_DESCRIPTOR_HEAP_DESC desc = Initializers::GetDescriptorHeapDescriptor( capacity, TRUE, heap_type );
if( capacity == 0
 || FAILED( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &heap->heap ) ) ) )
    {
    debug_assert_always();
    ReleaseRingHeap( heap );
    return( false );
    }

heap->capacity        = capacity;
heap->descriptor_size = Core::GetDevice()->GetDescriptorHandleIncrementSize( heap_type );
heap->heap_type       = heap_type;
heap->cpu_start       = heap->heap->GetCPUDescriptorHandleForHeapStart();
heap->gpu_start       = heap->heap->GetGPUDescriptorHandleForHeapStart();

return( true );

} /* RingHeap_Create() */


/*******************************************************************
*
*   RingHeap_Destroy()
*
*   DESCRIPTION:
*       Release the ring buffer descriptor heap's resources.
*
*******************************************************************/

void RingHeap_Destroy( RingHeap *heap )
{
ReleaseRingHeap( heap );

} /* RingHeap_Destroy() */


/*******************************************************************
*
*   RingHeap_GetSize()
*
*   DESCRIPTION:
*       Get the current size of the ring heap.
*
*******************************************************************/

uint32_t RingHeap_GetSize( const RingHeap *heap )
{
uint32_t ret = 0;
for( uint32_t i = 0; i < cnt_of_array( heap->frames ); i++ )
    {
    ret += heap->frames[ i ].count;
    }

return( ret );

} /* RingHeap_GetSize() */


/*******************************************************************
*
*   RingHeap_NextFrame()
*
*   DESCRIPTION:
*       Advance the ringbuffer by one frame.
*
*******************************************************************/

void RingHeap_NextFrame( RingHeap *heap )
{
heap->current_frame_index = ( heap->current_frame_index + 1 ) % Core::FRAME_COUNT;

RingHeapFrame *frame = &heap->frames[ heap->current_frame_index ];
frame->tail  = GetRingHeapHead( heap );
frame->count = 0;

RingHeapFrame *protect = &heap->frames[ ( heap->current_frame_index + 1 ) % Core::FRAME_COUNT ];
heap->tail = protect->tail;

} /* RingHeap_NextFrame() */


/*******************************************************************
*
*   ReleaseFreeHeap()
*
*   DESCRIPTION:
*        Release the free-list allocated descriptor heap's
*        resources.
*
*******************************************************************/

static void ReleaseFreeHeap( FreeHeap *heap )
{
TossTrash( &heap->heap );
free( heap->frees );

clr_struct( heap );

} /* ReleaseFreeHeap() */


/*******************************************************************
*
*   ReleaseRingHeap()
*
*   DESCRIPTION:
*        Release the ring allocated descriptor heap's resources.
*
*******************************************************************/

static void ReleaseRingHeap( RingHeap *heap )
{
TossTrash( &heap->heap );

clr_struct( heap );

} /* ReleaseRingHeap() */


} }/* namespace D3D12::Descriptor */