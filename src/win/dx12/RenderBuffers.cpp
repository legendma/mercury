#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ComUtilities.hpp"
#include "RenderBuffers.hpp"
#include "RenderEngine.hpp"
#include "RenderInitializers.hpp"
#include "Utilities.hpp"


#define INVALID_FRAME_INDEX         max_uint_value( uint32_t )


using namespace Render;
namespace Render { namespace Buffers
{

/*******************************************************************
*
*   UploadFrameIsAvailable()
*
*   DESCRIPTION:
*       Is the upload frame available for new use?
*
*******************************************************************/

static inline bool UploadFrameIsAvailable( Frame *frame )
{
return( frame->resource == NULL );

} /* UploadFrameIsAvailable() */


static ID3D12Resource * CreateUploadBuffer( const size_t aligned_sz, ID3D12Device* device );
static uint32_t         GetAvailableUploadFrame( Uploader *uploader );
static void             ReleaseUploader( Uploader *uploader );
static void             ResetUploadFrame( Frame* frame );
static void             WaitForUploadFrame( Frame *frame );


/*******************************************************************
*
*   Frame_EndUpload()
*
*   DESCRIPTION:
*       End an ongoing upload session and submit the copy to the
*       GPU.
*
*******************************************************************/

void Frame_EndUpload( Frame *frame )
{
debug_assert( frame->resource != NULL );
frame->resource->Unmap( 0, NULL );
_hr( frame->copy->Close() );

pthread_mutex_lock( &frame->uploader->mutex_queue );
ID3D12CommandList *submit = frame->copy;
frame->uploader->queue->ExecuteCommandLists( 1, &submit );
frame->fence_value = ++frame->uploader->fence_value;
_hr( frame->uploader->queue->Signal( frame->uploader->fence, frame->fence_value ) );
pthread_mutex_unlock( &frame->uploader->mutex_queue );

WaitForUploadFrame( frame );
ResetUploadFrame( frame );

} /* Frame_EndUpload() */


/*******************************************************************
*
*   Uploader_BeginUpload()
*
*   DESCRIPTION:
*       Begin a new upload session.
*
*******************************************************************/

Frame * Uploader_BeginUpload( const size_t aligned_sz, Uploader *uploader )
{
pthread_mutex_lock( &uploader->mutex_frame );
uint32_t frame_index = GetAvailableUploadFrame( uploader );
Frame *frame = &uploader->frames[ frame_index ];
frame->resource = (ID3D12Resource*)1;
pthread_mutex_unlock( &uploader->mutex_frame );

frame->resource = CreateUploadBuffer( aligned_sz, uploader->owner->device.ptr );
frame->resource->Map( 0, NULL, &frame->cpu_buffer );

_hr( frame->allocator->Reset() );
_hr( frame->copy->Reset( frame->allocator, NULL ) );

return( frame );

} /* Uploader_BeginUpload() */


/*******************************************************************
*
*   Uploader_Destroy()
*
*   DESCRIPTION:
*       Release the uploader's resources.
*
*******************************************************************/

void Uploader_Destroy( Uploader *uploader )
{
ReleaseUploader( uploader );
*uploader = {};

} /* Uploader_Destroy() */


/*******************************************************************
*
*   Uploader_Initialize()
*
*   DESCRIPTION:
*       Initialize a new upload context.
*
*******************************************************************/

bool Uploader_Initialize( Engine::_Engine *engine, Uploader *uploader )
{
*uploader = {};
uploader->owner = engine;

ID3D12Device *device = engine->device.ptr;
if( FAILED( device->CreateCommandQueue( Initializers::GetCommandQueueDescriptorUpload(), IID_PPV_ARGS( &uploader->queue ) ) ) )
    {
    ReleaseUploader( uploader );
    return( false );
    }

if( FAILED( device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &uploader->fence ) ) ) )
    {
    ReleaseUploader( uploader );
    return( false );
    }

for( uint32_t i = 0; i < cnt_of_array( uploader->frames ); i++ )
    {
    Frame *frame = &uploader->frames[ i ];
    frame->uploader = uploader;
    if( FAILED( device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS( &frame->allocator ) ) ) )
        {
        ReleaseUploader( uploader );
        return( false );
        }

    NameD3DIndex( frame->allocator, i, L"Uploader::FrameCommandAllocator" );

    if( FAILED( device->CreateCommandList( Initializers::NODE_MASK_SINGLE_GPU, D3D12_COMMAND_LIST_TYPE_COPY, frame->allocator, NULL, IID_PPV_ARGS( &frame->copy ) ) ) )
        {
        ReleaseUploader( uploader );
        return( false );
        }

    NameD3DIndex( frame->copy, i, L"Uploader::FrameCommandList" );

    frame->wait_event = ::CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
    if( frame->wait_event == NULL )
        {
        ReleaseUploader( uploader );
        return( false );
        }
    }

NameD3D( uploader->queue, L"Uploader::CommandQueue" );
NameD3D( uploader->fence, L"Uploader::Fence" );

return( true );

} /* Uploader_Initialize() */


/*******************************************************************
*
*   CreateUploadBuffer()
*
*   DESCRIPTION:
*       Create an upload buffer.
*
*******************************************************************/

static ID3D12Resource * CreateUploadBuffer( const size_t aligned_sz, ID3D12Device *device )
{
D3D12_RESOURCE_DESC desc = {};
desc.Dimension          = D3D12_RESOURCE_DIMENSION_BUFFER;
desc.Alignment          = 0;
desc.Width              = aligned_sz;
desc.Height             = 1;
desc.DepthOrArraySize   = 1;
desc.MipLevels          = 1;
desc.Format             = DXGI_FORMAT_UNKNOWN;
desc.SampleDesc.Count   = 1;
desc.SampleDesc.Quality = 0;
desc.Layout             = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
desc.Flags              = D3D12_RESOURCE_FLAG_NONE;

ID3D12Resource *ret = NULL;
if( FAILED( device->CreateCommittedResource( Initializers::GetUploadHeapProperties(), D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS( &ret ) ) ) )
    {
    hard_assert_always();
    return( NULL );
    }

return( ret );

} /* CreateUploadBuffer() */


/*******************************************************************
*
*   GetAvailableUploadFrame()
*
*   DESCRIPTION:
*       Obtain an upload frame that is not under use.
*
*******************************************************************/

static uint32_t GetAvailableUploadFrame( Uploader *uploader )
{
uint32_t ret = INVALID_FRAME_INDEX;
pthread_mutex_lock( &uploader->mutex_frame );
for( uint32_t i = 0; i < cnt_of_array( uploader->frames ); i++ )
    {
    Frame *frame = &uploader->frames[ i ];
    if( UploadFrameIsAvailable( frame ) )
        {
        ret = i;
        break;
        }
    }

while( ret == INVALID_FRAME_INDEX )
    {
    for( uint32_t i = 0; i < cnt_of_array( uploader->frames ); i++ )
        {
        uint32_t index = i % cnt_of_array( uploader->frames );
        Frame *frame = &uploader->frames[ index ];
        if( UploadFrameIsAvailable( frame ) )
            {
            ret = index;
            break;
            }

        sched_yield();
        }
    }

pthread_mutex_unlock( &uploader->mutex_frame );

return( ret );

} /* GetAvailableUploadFrame() */


/*******************************************************************
*
*   ReleaseUploader()
*
*   DESCRIPTION:
*       Release the uploader's resources.
*
*******************************************************************/

static void ReleaseUploader( Uploader *uploader )
{
ComSafeRelease( &uploader->fence );
ComSafeRelease( &uploader->queue );

for( uint32_t i = 0; i < cnt_of_array( uploader->frames ); i++ )
    {
    Frame *frame = &uploader->frames[ i ];
    ComSafeRelease( &frame->copy );
    ComSafeRelease( &frame->allocator );
    }

} /* ReleaseUploader() */


/*******************************************************************
*
*   ResetUploadFrame()
*
*   DESCRIPTION:
*       Restore an upload frame to an unused (available) state.
*
*******************************************************************/

static void ResetUploadFrame( Frame *frame )
{
frame->cpu_buffer = NULL;
frame->fence_value = 0;

/* do this last, as it triggers the frame being available */
ComSafeRelease( &frame->resource );

} /* ReleaseUploader() */


/*******************************************************************
*
*   WaitForUploadFrame()
*
*   DESCRIPTION:
*       Block the current thread until the given upload has
*       completed.
*
*******************************************************************/

static void WaitForUploadFrame( Frame *frame )
{
if( frame->uploader->fence->GetCompletedValue() < frame->fence_value )
    {
    frame->uploader->fence->SetEventOnCompletion( frame->fence_value, frame->wait_event );
    ::WaitForSingleObject( frame->wait_event, INFINITE );
    }

} /* WaitForUploadFrame() */


} }/* namespace Render::Upload */