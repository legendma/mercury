#include "RenderCore.hpp"

#define FRAME_TRASH_CAN_MAX_COUNT   ( 50 )

namespace Render { namespace Core
{

typedef struct _Window
    {
    uint32_t            width;
    uint32_t            height;
    HWND                handle;
    } Window;

typedef struct _SwapChain
    {

    } SwapChain;

typedef struct _Device
    {
    IMDevice           *device;
    IMDXGI             *dxgi;
    } Device;

typedef struct _Commands
    {
    IMCommandAllocator *global_allocator;
    IMGraphicsCommandList
                       *global_gfx;
    IMCommandQueue     *queue;
    ID3D12Fence        *frame_fence;
    uint64_t            last_submitted_frame;
    } Commands;

typedef struct _Thread
    {
    pthread_mutex_t     render_mutex;
    pthread_mutex_t     submit_mutex;
    pthread_t           render_thread;
    bool                is_paused;
    } Thread;

typedef struct _Frame
    {
    uint64_t            frame_count;
    pthread_mutex_t     trash_can_mutex;
    uint32_t            trash_can_count;
    IUnknown           *trash_can[ FRAME_TRASH_CAN_MAX_COUNT ];
    //IMCommandAllocator *command_allocator;
    uint8_t             frame_index;
    } Frame;

typedef struct _Core
    {
    Window              window;
    SwapChain           swap_chain;
    Device              device;
    Thread              thread;
    Frame               frames[ FRAME_COUNT ];
    uint8_t             current_frame;
    } Core;

static Core             _core;


/*******************************************************************
*
*   CurrentFrame()
*
*   DESCRIPTION:
*       Get the frame that is currently being built.
*
*******************************************************************/

static inline Frame * CurrentFrame()
{
return( &_core.frames[ _core.current_frame ] );

} /* CurrentFrame() */


/*******************************************************************
*
*   SweepFrameTrash()
*
*   DESCRIPTION:
*       Cleanup the trash in the given frame.  Frame must not be
*       in-flight.
*
*******************************************************************/

static inline void SweepFrameTrash( Frame *frame )
{
pthread_mutex_lock( &frame->trash_can_mutex );
for( uint32_t i = 0; i < frame->trash_can_count; i++ )
    {
    ComSafeRelease( &frame->trash_can[ i ] );
    }

frame->trash_can_count = 0;
pthread_mutex_unlock( &frame->trash_can_mutex );

} /* SweepFrameTrash() */


static bool CreateDevice();
static void Release();
static bool UpdateWindowExtent();


/*******************************************************************
*
*   AddTrashToCurrentFrame()
*
*   DESCRIPTION:
*       Add a piece of trash to the current frame's trash can.
*
*******************************************************************/

void AddTrashToCurrentFrame( IUnknown **trash )
{
Frame *frame = CurrentFrame();
pthread_mutex_lock( &frame->trash_can_mutex );
hard_assert( frame->trash_can_count < cnt_of_array( frame->trash_can ) );
frame->trash_can[ frame->trash_can_count++ ] = *trash;
*trash = NULL;
pthread_mutex_unlock( &frame->trash_can_mutex );

} /* AddTrashToCurrentFrame() */


/*******************************************************************
*
*   Destroy()
*
*   DESCRIPTION:
*       Destroy the render core.
*
*******************************************************************/

void Destroy()
{
Release();
clr_struct( &_core);

} /* Destroy() */


/*******************************************************************
*
*   GetDevice()
*
*   DESCRIPTION:
*       Get the device.
*
*******************************************************************/

IMDevice * GetDevice()
{
return( _core.device.device );

} /* GetDevice() */


/*******************************************************************
*
*   GetDXGI()
*
*   DESCRIPTION:
*       Get the DXGI interface.
*
*******************************************************************/

IMDXGI * GetDXGI()
{
return( _core.device.dxgi );

} /* GetDXGI() */


/*******************************************************************
*
*   Init()
*
*   DESCRIPTION:
*       Initialize the render core.
*
*******************************************************************/

bool Init()
{
clr_struct( &_core );

/* obtain handle to main application window */
_core.window.handle = ::GetActiveWindow();
if(_core.window.handle == NULL )
    {
    return( false );
    }
    
/* get the native dimensions of the application window */
if( !UpdateWindowExtent() )
    {
    return( false );
    }

if( !CreateDevice() )
    {
    debug_assert_always();
    Release();
    return( false );
    }


return( true );

} /* Init() */


/*******************************************************************
*
*   CreateDevice()
*
*   DESCRIPTION:
*       Create the DirectX Device.
*
*******************************************************************/

static bool CreateDevice()
{
UINT debug_flags = 0;
#if defined( _DEBUG )
ID3D12Debug            *debug_info;
if( SUCCEEDED( D3D12GetDebugInterface( IID_PPV_ARGS( &debug_info ) ) ) )
    {
    debug_info->EnableDebugLayer();
    ComSafeRelease( &debug_info );
    debug_flags = DXGI_CREATE_FACTORY_DEBUG;
    }
#endif

if( FAILED( CreateDXGIFactory2( debug_flags, IID_PPV_ARGS( &_core.device.dxgi ) ) ) )
    {
    return( false );
    }

if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &_core.device.device ) ) ) )
    {
    IDXGIAdapter       *warp;
    if( FAILED( _core.device.dxgi->EnumWarpAdapter( IID_PPV_ARGS( &warp ) ) ) )
        {
        ComSafeRelease( &warp );
        return( false );
        }

    if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &_core.device.device ) ) ) )
        {
        ComSafeRelease( &warp );
        return( false );
        }
    }

NameD3D( _core.device.device, L"Core::Device" );
      
return( true );

} /* CreateDevice() */


/*******************************************************************
*
*   Release()
*
*   DESCRIPTION:
*       Release the render resources.
*
*******************************************************************/

static void Release()
{
ComSafeRelease( &_core.device.device );
ComSafeRelease( &_core.device.dxgi );

} /* Release() */


/*******************************************************************
*
*   UpdateWindowExtent()
*
*   DESCRIPTION:
*       Get the width and height of the given window.
*
*******************************************************************/

static bool UpdateWindowExtent()
{
RECT rect;
if( !GetWindowRect( _core.window.handle, &rect ) )
    {
    return( false );
    }

_core.window.width  = (UINT)( rect.right  - rect.left );
_core.window.height = (UINT)( rect.bottom - rect.top  );

return( true );

} /* UpdateWindowExtent() */


} } /* namespace Render::Engine */