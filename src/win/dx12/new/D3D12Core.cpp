#include <string.h>

#include "D3D12Descriptor.hpp"
#include "D3D12Core.hpp"
#include "D3D12Initializers.hpp"
#include "D3D12Pipelines.hpp"
#include "D3D12Texture.hpp"
#include "D3D12Upload.hpp"
#include "ShaderCache.hpp"
#include "Utilities.hpp"

#define FRAME_TRASH_CAN_MAX_COUNT   ( 50 )
#define ENGINE_SHADERS_MAX_COUNT    ( 50 )
#define ENGINE_SHADERS_STORAGE_SZ   ( 1024 * 1024 )

#define RTV_DESCRIPTOR_MAX_COUNT    ( 50 )
#define DSV_DESCRIPTOR_MAX_COUNT    ( 50 )
#define SRV_DESCRIPTOR_MAX_COUNT    ( 100000 )
#define SRV_UAV_CBV_PER_FRAME_MAX_COUNT \
                                    ( 100000 )
#define SRV_UAV_CBV_GPU_VISIBLE_MAX_DESCRIPTOR_COUNT \
                                    ( FRAME_COUNT * SRV_UAV_CBV_PER_FRAME_MAX_COUNT )

namespace D3D12 { namespace Core
{

typedef struct _Window
    {
    uint32_t            width;
    uint32_t            height;
    HWND                handle;
    } Window;

typedef struct _SwapChain
    {
    IMSwapChain        *swap_chain;
    Texture::RenderBuffer
                        backbuffers[ BACKBUFFER_COUNT ];
    uint8_t             current_backbuffer;
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
                       *gfx;
    IMCommandQueue     *queue;
    IMFence            *frame_fence;
    uint64_t            last_submitted_frame;
    HANDLE              wait_event;
    } Commands;

typedef struct _Frame
    {
    uint64_t            frame_count;
    pthread_mutex_t     trash_can_mutex;
    uint32_t            trash_can_count;
    IUnknown           *trash_can[ FRAME_TRASH_CAN_MAX_COUNT ];
    uint8_t             frame_index;
    IMCommandAllocator *private_allocator;
    } Frame;

typedef struct _Descriptors
    {
    Descriptor::FreeHeap
                        dsvs;
    Descriptor::FreeHeap
                        rtvs;
    Descriptor::FreeHeap
                        srvs;
    Descriptor::RingHeap
                        gpu_srvs;
    } Descriptors;

typedef struct _Core
    {
    Window              window;
    SwapChain           swap_chain;
    Device              device;
    Frame               frames[ FRAME_COUNT ];
    uint8_t             current_frame;
    Commands            commands;
    Descriptors         descriptors;
    Render::Shader::Cache
                        engine_shaders;
    } Core;

static Core             _core;


/*******************************************************************
*
*   CurrentBackbuffer()
*
*   DESCRIPTION:
*       Get the backbuffer currently being rendered to.
*
*******************************************************************/

static inline Texture::RenderBuffer * CurrentBackbuffer()
{
return( &_core.swap_chain.backbuffers[ _core.swap_chain.current_backbuffer ] );

} /* CurrentBackbuffer() */


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


static bool CreateCommandObjects();
static bool CreateDescriptors();
static bool CreateDevice();
static bool CreateFrames();
static bool CreateSwapChain();
static bool LoadEngineShaders();
static void FlushCommandQueue();
static void Release();
static bool UpdateWindowExtent();
static bool WaitForFrameToFinish( uint64_t frame_num );


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
*   BeginNewFrame()
*
*   DESCRIPTION:
*       Start a new render frame.
*
*******************************************************************/

void BeginNewFrame()
{
Frame *new_frame = CurrentFrame();
hard_assert( WaitForFrameToFinish( new_frame->frame_count ) );
SweepFrameTrash( new_frame );
Descriptor::RingHeap_NextFrame( &_core.descriptors.gpu_srvs );

} /* BeginNewFrame() */


/*******************************************************************
*
*   CreateHeap()
*
*   DESCRIPTION:
*       Create a manual heap for placing resources.
*
*******************************************************************/

IMHeap * CreateHeap( const uint64_t sz )
{
IMHeap *ret = NULL;

D3D12_HEAP_DESC desc = Initializers::GetHeapDescriptor( sz );
if( FAILED( _core.device.device->CreateHeap( &desc, IID_PPV_ARGS( &ret ) ) ) )
    {
    hard_assert_always();
    return( NULL );
    }

return( ret );

} /* CreateHeap() */


/*******************************************************************
*
*   CreatePlacedBuffer()
*
*   DESCRIPTION:
*       Create a placed buffer in the given heap.
*
*******************************************************************/

IMResource * CreatePlacedBuffer( const uint64_t aligned_offset, const uint64_t aligned_sz, D3D12_RESOURCE_STATES initial_state, IMHeap *heap )
{
IMResource *ret = NULL;

D3D12_RESOURCE_DESC desc = Initializers::GetBufferResourceDescriptor( aligned_sz );
if( FAILED( _core.device.device->CreatePlacedResource( heap, aligned_offset, &desc, initial_state, NULL, IID_PPV_ARGS( &ret ) ) ) )
    {
    hard_assert_always();
    return( NULL );
    }

return( ret );

} /* CreatePlacedBuffer() */


/*******************************************************************
*
*   CreateUploadBuffer()
*
*   DESCRIPTION:
*       Create an upload buffer of the requested size.
*
*******************************************************************/

IMResource * CreateUploadBuffer( const uint64_t sz )
{
IMResource *ret = NULL;

D3D12_RESOURCE_DESC desc = Initializers::GetBufferResourceDescriptor( sz );
if( FAILED( _core.device.device->CreateCommittedResource( Initializers::GetUploadHeapProperties(), D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS( &ret ) ) ) )
    {
    hard_assert_always();
    return( NULL );
    }

return( ret );

} /* CreateUploadBuffer() */


/*******************************************************************
*
*   ExecuteCommandList()
*
*   DESCRIPTION:
*       Destroy the render core.
*
*******************************************************************/

void ExecuteCommandList( ID3D12CommandList *submission )
{
_core.commands.queue->ExecuteCommandLists( 1, &submission );

} /* ExecuteCommandList() */


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
*   GetDSVHeap()
*
*   DESCRIPTION:
*       Get the DSV descriptor heap.
*
*******************************************************************/

Descriptor::FreeHeap * GetDSVHeap()
{
return( &_core.descriptors.dsvs );

} /* GetDSVHeap() */


/*******************************************************************
*
*   GetGPUSRVHeap()
*
*   DESCRIPTION:
*       Get the GPU visible SRV desriptor heap.
*
*******************************************************************/

Descriptor::RingHeap * GetGPUSRVHeap()
{
return( &_core.descriptors.gpu_srvs );

} /* GetGPUSRVHeap() */


/*******************************************************************
*
*   GetRTVHeap()
*
*   DESCRIPTION:
*       Get the RTV descriptor heap.
*
*******************************************************************/

Descriptor::FreeHeap * GetRTVHeap()
{
return( &_core.descriptors.rtvs );

} /* GetRTVHeap() */


/*******************************************************************
*
*   GetSRVHeap()
*
*   DESCRIPTION:
*       Get the SRV/UAV/CBV descriptor staging heap.
*
*******************************************************************/

Descriptor::FreeHeap * GetSRVHeap()
{
return( &_core.descriptors.srvs );

} /* GetSRVHeap() */


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
if( _core.window.handle == NULL )
    {
    return( false );
    }

/* get the file load for the engine shaders started right away */
if( !LoadEngineShaders() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

/* create the windows wait event */
HANDLE blocking = ::CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
if( !blocking )
    {
    debug_assert_always();
    Release();
    return( false );
    }
    
/* get the native dimensions of the application window */
if( !UpdateWindowExtent() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

if( !CreateDevice() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

if( !CreateCommandObjects() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

if( !CreateDescriptors() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

if( !CreateFrames() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

if( !CreateSwapChain() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

if( !Upload::Init() )
    {
    debug_assert_always();
    Release();
    return( false );
    }

return( true );

} /* Init() */


/*******************************************************************
*
*   Present()
*
*   DESCRIPTION:
*       Present the given render buffer to the screen.
*
*******************************************************************/

void Present( const Texture::RenderBuffer *buffer )
{
static const Color4f clear_color = { 0.0f, 0.0f, 0.0f, 0.0f };

Frame *frame = CurrentFrame();
_hr( _core.commands.gfx->Reset( frame->private_allocator, NULL ) );

Texture::RenderBuffer *backbuffer = CurrentBackbuffer();
D3D12_RESOURCE_BARRIER to_render_target = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET, backbuffer->texture.resource );
_core.commands.gfx->ResourceBarrier( 1, &to_render_target );
_core.commands.gfx->OMSetRenderTargets( 1, &backbuffer->rtv->hcpu, TRUE, NULL );
_core.commands.gfx->ClearRenderTargetView( backbuffer->rtv->hcpu, clear_color.f, 0, NULL );

// TODO - Render given buffer to screen

D3D12_RESOURCE_BARRIER to_present = Initializers::GetResourceTransition( D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT, backbuffer->texture.resource );
_core.commands.gfx->ResourceBarrier( 1, &to_present );
_hr( _core.commands.gfx->Close() );
ID3D12CommandList *command_list = _core.commands.gfx;
ExecuteCommandList( command_list );

_hr( _core.swap_chain.swap_chain->Present( 0, 0 ) );
_core.swap_chain.current_backbuffer = (uint8_t)_core.swap_chain.swap_chain->GetCurrentBackBufferIndex();
backbuffer = NULL;

frame->frame_count = ++_core.commands.last_submitted_frame;
_hr( _core.commands.queue->Signal( _core.commands.frame_fence, frame->frame_count ) );
frame = NULL;
++_core.current_frame %= cnt_of_array(_core.frames);

} /* Present() */


/*******************************************************************
*
*   Shutdown()
*
*   DESCRIPTION:
*       Shutdown the render core.
*
*******************************************************************/

void Shutdown()
{
Release();
clr_struct( &_core);

} /* Shutdown() */


/*******************************************************************
*
*   CreateCommandObjects()
*
*   DESCRIPTION:
*       Create the objects needed to record and submit render
*       commands.
*
*******************************************************************/

static bool CreateCommandObjects()
{
/* fence */
if( FAILED( _core.device.device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &_core.commands.frame_fence ) ) ) )
    {
    return(false);
    }

NameD3D( _core.commands.frame_fence, L"Core::Fence" );

/* command queue */
D3D12_COMMAND_QUEUE_DESC desc = {};
desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
if( FAILED( _core.device.device->CreateCommandQueue1( &desc, CREATOR_ID, IID_PPV_ARGS( &_core.commands.queue ) ) ) )
    {
    return( false );
    }

NameD3D( _core.commands.queue, L"Core::CommandQueue" );

/* global allocator - we'll use to the issue 'important' commands which fall outside the realm of per-frame */
if( FAILED(_core.device.device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &_core.commands.global_allocator ) ) ) )
    {
    return( false );
    }

NameD3D( _core.commands.global_allocator, L"Core::CommandAllocator" );

/* command recording list */
if( FAILED( _core.device.device->CreateCommandList( GPU_NODE_MASK, D3D12_COMMAND_LIST_TYPE_DIRECT, _core.commands.global_allocator, NULL, IID_PPV_ARGS( &_core.commands.gfx ) ) ) )
    {
    return( false );
    }

NameD3D( _core.commands.gfx, L"Core::CommandList" );

return( true );

} /* CreateCommandObjects() */


/*******************************************************************
*
*   CreateDescriptors()
*
*   DESCRIPTION:
*       Create the descriptor heaps.
*
*******************************************************************/

static bool CreateDescriptors()
{
/* RTV heap */
if( !Descriptor::FreeHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_RTV, RTV_DESCRIPTOR_MAX_COUNT, &_core.descriptors.rtvs ) )
    {
    debug_assert_always();
    return( false );
    }

NameD3D( _core.descriptors.rtvs.heap, L"Core::RTVHeap" );

/* DSV heap */
if( !Descriptor::FreeHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_DSV, DSV_DESCRIPTOR_MAX_COUNT, &_core.descriptors.dsvs ) )
    {
    debug_assert_always();
    return( false );
    }

NameD3D( _core.descriptors.dsvs.heap, L"Core::DSVHeap" );

/* SRV/UAV/CBV staging heap */
if( !Descriptor::FreeHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SRV_DESCRIPTOR_MAX_COUNT, &_core.descriptors.srvs ) )
    {
    debug_assert_always();
    return( false );
    }

NameD3D( _core.descriptors.srvs.heap, L"Core::SRVStagingHeap" );

/* SRV/UAV/CBV GPU visible heap */
if( !Descriptor::RingHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, SRV_UAV_CBV_GPU_VISIBLE_MAX_DESCRIPTOR_COUNT, &_core.descriptors.gpu_srvs ) )
    {
    debug_assert_always();
    return( false );
    }

NameD3D( _core.descriptors.gpu_srvs.heap, L"Core::SRVGPUVisibleHeap" );

return( true );

} /* CreateDescriptors() */


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

if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS( &_core.device.device ) ) ) )
    {
    IDXGIAdapter       *warp;
    if( FAILED( _core.device.dxgi->EnumWarpAdapter( IID_PPV_ARGS( &warp ) ) ) )
        {
        ComSafeRelease( &warp );
        return( false );
        }

    if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS( &_core.device.device ) ) ) )
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
*   CreateFrames()
*
*   DESCRIPTION:
*       Create the frame resources.
*
*******************************************************************/

static bool CreateFrames()
{
for( uint32_t i = 0; i < cnt_of_array( _core.frames ); i++ )
    {
    Frame *frame = &_core.frames[ i ];
    if( FAILED( _core.device.device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &frame->private_allocator ) ) ) )
        {
        return( false );
        }

    NameD3DIndex( frame->private_allocator, i, L"Core::FrameCommandAllocator" );
    pthread_mutex_init( &frame->trash_can_mutex, NULL );
    }
      
return( true );

} /* CreateDevice() */


/*******************************************************************
*
*   CreateSwapChain()
*
*   DESCRIPTION:
*       Create the DirectX device.
*
*******************************************************************/

static bool CreateSwapChain()
{
if( !Descriptor::FreeHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_RTV, BACKBUFFER_COUNT, &_core.descriptors.rtvs ) )
    {
    return( false );
    }

IDXGISwapChain1 *swap_chain = NULL;
DXGI_SWAP_CHAIN_DESC1 desc = Initializers::GetSwapChainDescriptor( _core.window.width, _core.window.height, DEFAULT_RENDER_TARGET_FORMAT, _core.window.handle );
if( FAILED( _core.device.dxgi->CreateSwapChainForHwnd(_core.device.device, _core.window.handle, &desc, NULL, NULL, &swap_chain ) )
 || FAILED( swap_chain->QueryInterface( IID_PPV_ARGS( &_core.swap_chain.swap_chain ) ) ) )
    {
    ComSafeRelease( &swap_chain );
    return( false );
    }

ComSafeRelease( &swap_chain );
_core.swap_chain.current_backbuffer = _core.swap_chain.swap_chain->GetCurrentBackBufferIndex();

for( uint32_t i = 0; i < cnt_of_array( _core.swap_chain.backbuffers ); i++ )
    {
    Texture::TextureCreateParams params = {};
    D3D12_RESOURCE_DESC resource_desc = Initializers::GetTexture2DResourceDescriptor( (uint16_t)desc.Width, (uint16_t)desc.Height, Initializers::TEXTURE_USAGE_RENDER_TARGET, desc.Format );
    params.desc = &resource_desc;
    if( FAILED( _core.swap_chain.swap_chain->GetBuffer( (UINT)i, IID_PPV_ARGS( &params.resource ) ) ) )
        {
        return( false );
        }

    Texture::RenderBuffer *backbuffer = &_core.swap_chain.backbuffers[ i ];
    Texture::RenderBuffer_Create( &params, backbuffer );
    NameD3DIndex( backbuffer->texture.resource, i, L"Engine::SwapChain" );
    }
      
return( true );

} /* CreateSwapChain() */


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
WaitForFrameToFinish( _core.commands.last_submitted_frame );

} /* FlushCommandQueue() */


/*******************************************************************
*
*   LoadEngineShaders()
*
*   DESCRIPTION:
*       Get the width and height of the given window.
*
*******************************************************************/

static bool LoadEngineShaders()
{
if( !Render::Shader::Cache_Create( ENGINE_SHADERS_MAX_COUNT, ENGINE_SHADERS_STORAGE_SZ, &_core.engine_shaders ) )
    {
    return( false );
    }

for( uint32_t i = 0; i < cnt_of_array( Pipelines::ENGINE_SHADER_NAMES ); i++ )
    {
    AssetFileAssetId shader_id = AssetFile_MakeAssetIdFromName( Pipelines::ENGINE_SHADER_NAMES[ i ], (uint32_t)strlen( Pipelines::ENGINE_SHADER_NAMES[ i ] ) );
    do_debug_assert( Render::Shader::Cache_AddAsyncRequest( shader_id, &_core.engine_shaders ) );
    }

Render::Shader::Cache_LoadAsync( "EngineShdrLoad",  & _core.engine_shaders);

return( true );

} /* LoadEngineShaders() */


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
Upload::Shutdown();
FlushCommandQueue();

Descriptor::RingHeap_Destroy( &_core.descriptors.gpu_srvs );
Descriptor::FreeHeap_Destroy( &_core.descriptors.rtvs );
Descriptor::FreeHeap_Destroy( &_core.descriptors.dsvs );
Descriptor::FreeHeap_Destroy( &_core.descriptors.srvs );

for( uint32_t i = 0; i < cnt_of_array( _core.frames ); i++ )
    {
    Frame *frame = &_core.frames[ i ];
    SweepFrameTrash( frame );
    ComSafeRelease( &frame->private_allocator );
    pthread_mutex_destroy( &frame->trash_can_mutex );
    }

for( uint32_t i = 0; i < cnt_of_array( _core.swap_chain.backbuffers ); i++ )
    {
    Texture::RenderBuffer_Destroy( &_core.swap_chain.backbuffers[ i ] );
    }

ComSafeRelease( &_core.swap_chain.swap_chain );
ComSafeRelease( &_core.commands.frame_fence );
ComSafeRelease( &_core.commands.gfx );
ComSafeRelease( &_core.commands.global_allocator );
ComSafeRelease( &_core.commands.queue );
ComSafeRelease( &_core.device.device );
ComSafeRelease( &_core.device.dxgi );

::CloseHandle(  _core.commands.wait_event );
Render::Shader::Cache_Destroy( &_core.engine_shaders );

clr_struct( &_core );

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
if( FAILED( _core.commands.frame_fence->SetEventOnCompletion( frame_num, _core.commands.wait_event ) ) )
    {
    debug_assert_always();
    return( false );
    }

::WaitForSingleObject( _core.commands.wait_event, INFINITE );

return( true );

} /* WaitForFrameToFinish() */


} } /* namespace D3D12::Engine */