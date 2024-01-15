#include <stdint.h>
#define WIN32_LEAN_AND_MEAN 
#include <windows.h>
#include <pthread.h>

#include "D3D12Core.hpp"
#include "D3D12Initializers.hpp"
#include "D3D12Upload.hpp"
#include "D3D12Types.hpp"
#include "ResourceLoader.hpp"
#include "Utilities.hpp"

#define MAX_UPLOAD_FRAMES           ( 5 )
#define FRAME_RESERVE_SENTINEL      ( 0xffffffff )
                              

namespace D3D12 { namespace Upload
{

typedef struct _Copy
    {
    Core::IMCommandQueue
                       *command_queue;
    Core::IMFence      *fence;
    uint64_t            last_submitted_frame;
    HANDLE              wait_event;
    Frame               frames[ MAX_UPLOAD_FRAMES ];
    pthread_mutex_t     server_mutex;
    bool                is_running;
    } Copy;


static Copy             _copy;
static uint32_t         _init = 0;


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
return( frame->upload_count == 0 );

} /* UploadFrameIsAvailable() */


static void    FlushCommandQueue();
static void    Release();
static void    ResetFrame( Frame *frame );
static bool    WaitForFrameToFinish( uint64_t frame_num );


/*******************************************************************
*
*   AllocateUpload()
*
*   DESCRIPTION:
*       Generate a new upload buffer.
*
*******************************************************************/

Core::IMResource * AllocateUpload( const uint64_t sz, Frame *frame )
{
uint32_t upload_count = frame->upload_count;
if( upload_count == FRAME_RESERVE_SENTINEL )
    {
    upload_count = 0;
    }

if( upload_count >= cnt_of_array( frame->uploads ) )
    {
    /* out of uploads */
    return( NULL );
    }

Core::IMResource *ret = Core::CreateUploadBuffer( sz );
if( !ret )
    {
    return( NULL );
    }

NameD3DIndex( ret, upload_count, L"UploadFrame::UploadBuffer" );
frame->uploads[ upload_count++ ] = ret;
frame->upload_count = upload_count;

return( ret );

} /* AllocateUpload() */


/*******************************************************************
*
*   BeginFrame()
*
*   DESCRIPTION:
*       Obtain a new upload frame.
*
*******************************************************************/

Frame * BeginFrame()
{
Frame *ret = NULL;
while( !ret )
    {
    pthread_mutex_lock( &_copy.server_mutex );
    if( !_copy.is_running )
        {
        pthread_mutex_unlock( &_copy.server_mutex );
        return( NULL );
        }

    for( uint32_t i = 0; i < cnt_of_array( _copy.frames ); i++ )
        {
        Frame *frame = &_copy.frames[ i ];
        if( UploadFrameIsAvailable( frame ) )
            {
            ret = frame;
            ret->upload_count = FRAME_RESERVE_SENTINEL;
            pthread_mutex_lock( &ret->client_mutex );
            break;
            }
        }

    pthread_mutex_unlock( &_copy.server_mutex );
    if( !ret )
        {
        sched_yield();
        }
    }

_hr( ret->command_allocator->Reset() );
_hr( ret->command_list->Reset( ret->command_allocator, NULL ) );

return( ret );

} /* BeginFrame() */


/*******************************************************************
*
*   CancelFrame()
*
*   DESCRIPTION:
*       Abort an upload frame.
*
*******************************************************************/

void CancelFrame( Frame *frame )
{
pthread_mutex_lock( &_copy.server_mutex );
if( !_copy.is_running )
    {
    pthread_mutex_unlock( &_copy.server_mutex );
    return;
    }

_hr( frame->command_list->Close() );

for( uint32_t i = 0; frame->upload_count != FRAME_RESERVE_SENTINEL && i < frame->upload_count; i++ )
    {
    ComSafeRelease( &frame->uploads[ i ] );
    }

frame->upload_count = 0;

pthread_mutex_unlock( &_copy.server_mutex );
pthread_mutex_unlock( &frame->client_mutex );

} /* EndFrame() */


/*******************************************************************
*
*   EndFrame()
*
*   DESCRIPTION:
*       Submit the frame's command list.
*
*******************************************************************/

void EndFrame( Frame *frame )
{
pthread_mutex_lock( &_copy.server_mutex );
if( !_copy.is_running )
    {
    pthread_mutex_unlock( &_copy.server_mutex );
    return;
    }

_hr( frame->command_list->Close() );

frame->fence_value = ++_copy.last_submitted_frame;
ID3D12CommandList *submission = frame->command_list;
_copy.command_queue->ExecuteCommandLists( 1, &submission );
_hr( _copy.command_queue->Signal( _copy.fence, frame->fence_value ) );

pthread_mutex_unlock( &_copy.server_mutex );
pthread_mutex_unlock( &frame->client_mutex );
WaitForFrameToFinish( frame->fence_value );
ResetFrame( frame );

} /* EndFrame() */


/*******************************************************************
*
*   Init()
*
*   DESCRIPTION:
*       Initialize the copy engine.
*
*******************************************************************/

bool Init()
{
if( _init == MAGIC_INIT )
    {
    debug_assert_always();
    return( true );
    }

_copy = {};
Core::IMDevice *device = Core::GetDevice();

/* create the window's wait event */
HANDLE blocking = ::CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
if( !blocking )
    {
    debug_assert_always();
    Release();
    return( false );
    }

for( uint32_t i = 0; i < cnt_of_array( _copy.frames ); i++ )
    {
    Frame *frame = &_copy.frames[ i ];
    frame->client_mutex = PTHREAD_MUTEX_INITIALIZER;
    if( FAILED( device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_COPY, IID_PPV_ARGS( &frame->command_allocator ) ) ) )
        {
        debug_assert_always();
        Release();
        return( false );
        }

    NameD3DIndex( frame->command_allocator, i, L"Upload::FrameCommandAllocator" );

    if( FAILED( device->CreateCommandList( Core::GPU_NODE_MASK, D3D12_COMMAND_LIST_TYPE_COPY, frame->command_allocator, NULL, IID_PPV_ARGS( &frame->command_list ) ) ) )
        {
        debug_assert_always();
        Release();
        return( false );
        }

    frame->command_list->Close();

    NameD3DIndex( frame->command_list, i, L"Upload::FrameCommandList" );
    }

if( FAILED( device->CreateCommandQueue1( Initializers::GetCommandQueueDescriptorUpload(), Core::CREATOR_ID, IID_PPV_ARGS( &_copy.command_queue ) ) ) )
    {
    debug_assert_always();
    Release();
    return( false );
    }

NameD3D( _copy.command_queue, L"Upload::CommandQueue" );

if( FAILED( device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &_copy.fence ) ) ) )
    {
    debug_assert_always();
    Release();
    return( false );
    }

NameD3D( _copy.fence, L"Upload::Fence" );
_copy.server_mutex = PTHREAD_MUTEX_INITIALIZER;
_copy.is_running = true;

_init = MAGIC_INIT;

return( true );

} /* Init() */


/*******************************************************************
*
*   Shutdown()
*
*   DESCRIPTION:
*       Initialize the copy engine.
*
*******************************************************************/

void Shutdown()
{
if( _init != MAGIC_INIT )
    {
    debug_assert_always();
    return;
    }

Release();
_init = 0;

} /* Shutdown() */


/*******************************************************************
*
*   FlushCommandQueue()
*
*   DESCRIPTION:
*       Ensure any outstanding GPU commands are complete.
*
*******************************************************************/

static void FlushCommandQueue()
{
WaitForFrameToFinish( _copy.last_submitted_frame );

} /* FlushCommandQueue() */


/*******************************************************************
*
*   Release()
*
*   DESCRIPTION:
*       Release the resources used by this module.
*
*******************************************************************/

static void Release()
{
pthread_mutex_lock( &_copy.server_mutex );
_copy.is_running = false;
pthread_mutex_unlock( &_copy.server_mutex );

for( uint32_t i = 0; i < cnt_of_array( _copy.frames ); i++ )
    {
    Frame *frame = &_copy.frames[ i ];

    pthread_mutex_lock( &frame->client_mutex );
    pthread_mutex_unlock( &frame->client_mutex );

    ComSafeRelease( &frame->command_list );
    ComSafeRelease( &frame->command_allocator );
    pthread_mutex_destroy( &frame->client_mutex );
    }

FlushCommandQueue();
ComSafeRelease( &_copy.fence );
ComSafeRelease( &_copy.command_queue );
::CloseHandle( _copy.wait_event );

pthread_mutex_destroy( &_copy.server_mutex );
_copy = {};

} /* Release() */


/*******************************************************************
*
*   ResetFrame()
*
*   DESCRIPTION:
*       Free the frame's upload buffer so that it can be re-used.
*
*******************************************************************/

static void ResetFrame( Frame *frame )
{
pthread_mutex_lock( &_copy.server_mutex );
for( uint32_t i = 0; i < frame->upload_count; i++ )
    {
    ComSafeRelease( &frame->uploads[ i ] );
    }

frame->upload_count = 0;
pthread_mutex_unlock( &_copy.server_mutex );

} /* ResetFrame() */


/*******************************************************************
*
*   WaitForFrameStart()
*
*   DESCRIPTION:
*       Wait for the given frame to become available.
*
*******************************************************************/

static bool WaitForFrameToFinish( uint64_t frame_num )
{
if( FAILED( _copy.fence->SetEventOnCompletion( frame_num, _copy.wait_event ) ) )
    {
    debug_assert_always();
    return( false );
    }

::WaitForSingleObject( _copy.wait_event, INFINITE );

return( true );

} /* WaitForFrameToFinish() */


} }/* namespace D3D12::Upload */