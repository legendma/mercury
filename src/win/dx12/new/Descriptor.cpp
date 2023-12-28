#include "RenderCore.hpp"
#include "Descriptor.hpp"

namespace Render { namespace Descriptor
{

    
void             Heap_NextFrame( Heap *heap );
void             Heap_Reset( Heap *heap );

/*******************************************************************
*
*   Heap_Allocate()
*
*   DESCRIPTION:
*       Allocate a number of descriptor handles from the given heap.
*
*******************************************************************/

Handle Heap_Allocate( Heap *heap )
{
Handle ret = {};
Heap_AllocateBatch( 1, 1, &ret, heap );

return( ret );

} /* Heap_Allocate() */


/*******************************************************************
*
*   Heap_AllocateBatch()
*
*   DESCRIPTION:
*       Allocate a number of descriptor handles from the given heap.
*
*******************************************************************/

bool Heap_AllocateBatch( const uint32_t allocate_cnt, uint32_t out_capacity, Handle *out, Heap *heap )
{
uint32_t remaining = heap->capacity - heap->head;
if( remaining < allocate_cnt )
    {
    if( heap->is_ring )
        {
        /* Ring Allocator */
        if( heap->frames_since_wrap < Render::Core::FRAME_COUNT )
            {
            assert( false );
            return( false );
            }

        heap->frames_since_wrap = 0;
        heap->head = 0;
        }
    else
        {
        /* Linear Allocator */
        /* Asking count over capacity, and wraps not allowed */
        assert( false );
        return( false );
        }

    }

for( uint32_t i = 0; i < allocate_cnt; i++ )
    {
    out[ i ].cpu_hndl.ptr = heap->cpu_start.ptr + ( i + heap->head ) * heap->descriptor_size;
    out[ i ].gpu_hndl.ptr = heap->gpu_start.ptr + ( i + heap->head ) * heap->descriptor_size;
    }

heap->head += allocate_cnt;

return( true );

} /* Heap_AllocateBatch() */


/*******************************************************************
*
*   Heap_Create()
*
*   DESCRIPTION:
*       Create a descriptor heap by allocating it within the given
*       device.
*
*******************************************************************/

bool Heap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool is_ring, const uint32_t capacity, Core::IMDevice *device, Heap *heap )
{
*heap = {};
heap->capacity          = capacity;
heap->type              = type;
heap->is_ring           = is_ring;
heap->is_shader_visible = ( type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                         || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
heap->descriptor_size   = device->GetDescriptorHandleIncrementSize( heap->type );

D3D12_DESCRIPTOR_HEAP_DESC desc = Initializers::GetHeapDescriptor( capacity, heap->is_shader_visible, type );
if( FAILED( device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &heap->heap ) ) ) )
    {
    assert( false );
    return( false );
    }

heap->cpu_start = heap->heap->GetCPUDescriptorHandleForHeapStart();
if( heap->is_shader_visible )
    {
    heap->gpu_start = heap->heap->GetGPUDescriptorHandleForHeapStart();
    }

return( true );

} /* Heap_Create() */


/*******************************************************************
*
*   DescriptorHeap_Destroy()
*
*   DESCRIPTION:
*       Destroy the given descriptor heap.
*
*******************************************************************/

void Heap_Destroy( const bool should_defer, Heap *heap )
{
if( should_defer )
    {
    TossTrash( &heap->heap );
    }
else
    {
    ComSafeRelease( &heap->heap );
    }

*heap = {};

} /* DescriptorHeap_Destroy() */


/*******************************************************************
*
*   Heap_Reset()
*
*   DESCRIPTION:
*       Reset all allocations of the given descriptor heap.
*
*******************************************************************/

void Heap_Reset( Heap *heap )
{
heap->head = 0;

} /* Heap_Reset() */

} }/* namespace Render::Descriptor */