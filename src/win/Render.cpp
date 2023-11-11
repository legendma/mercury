#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dxgi1_4.h>
#include <d3d12sdklayers.h>
#include <winbase.h>
#include <winuser.h>

#include "ComUtilities.hpp"
#include "Render.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"

using namespace ECS;

#define SWAP_CHAIN_DOUBLE_BUFFER    ( 2 )
#define NODE_MASK_SINGLE_GPU        ( 0 )

typedef enum _BackbufferSwapResult
    {
    BACKBUFFER_SWAP_RESULT_PRESENTATION,
    BACKBUFFER_SWAP_RESULT_BACKBUFFER,
    } BackbufferSwapResult;

typedef struct _DX12Render
    {
    ID3D12Device       *device;
    IDXGIFactory4      *dxgi_factory;
    UINT                descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_NUM_TYPES ];
    HWND                hwnd;
    ID3D12Fence        *fence;
    uint64_t            fence_current;
    ID3D12GraphicsCommandList
                       *gfx;
    ID3D12CommandQueue *command_queue;
    ID3D12CommandAllocator
                       *command_allocator;
    IDXGISwapChain     *swap_chain;
    ID3D12DescriptorHeap
                       *render_target_heap;
    DXGI_FORMAT         render_target_format;
    ID3D12DescriptorHeap
                       *depth_stencil_heap;
    ID3D12Resource     *depth_stencil_buffer;
    DXGI_FORMAT         depth_stencil_format;
    ID3D12Resource     *backbuffers[ SWAP_CHAIN_DOUBLE_BUFFER ];
    uint8_t             backbuffer_current;
    UINT                window_width;
    UINT                window_height;
    UINT                msaa_quality;
    UINT                msaa_count;
    } DX12Render;


/*******************************************************************
*
*   GetCurrentBackbuffer()
*
*   DESCRIPTION:
*       Get the current backbuffer texture.
*
*******************************************************************/

static inline ID3D12Resource * GetCurrentBackbuffer( DX12Render *render )
{
return( render->backbuffers[ render->backbuffer_current ] );

} /* GetCurrentBackbuffer() */


/*******************************************************************
*
*   GetCurrentBackbufferView()
*
*   DESCRIPTION:
*       Get the render target view for the current backbuffer.
*
*******************************************************************/

static inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackbufferView( DX12Render *render )
{
D3D12_CPU_DESCRIPTOR_HANDLE ret = render->render_target_heap->GetCPUDescriptorHandleForHeapStart();
ret.ptr += render->backbuffer_current * render->descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];

return( ret );

} /* GetCurrentBackbufferView() */


/*******************************************************************
*
*   GetDepthStencilView()
*
*   DESCRIPTION:
*       Get the depth/stencil view..
*
*******************************************************************/

static inline D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView( DX12Render *render )
{
return( render->depth_stencil_heap->GetCPUDescriptorHandleForHeapStart() );

} /* GetDepthStencilView() */


static DX12Render *           AsDX12Render( Universe *universe );
static void                   ClearBackbuffer( const Color4f clear_color, DX12Render *render );
static void                   ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, DX12Render *render );
static bool                   CreateCommandObjects( DX12Render *render );
static bool                   CreateDepthStencil( DX12Render *render );
static bool                   CreateDescriptorHeaps( DX12Render *render );
static bool                   CreateDevice( DX12Render *render );
static bool                   CreateRenderTargetViews( DX12Render *render );
static bool                   CreateSwapChain( DX12Render *render );
static void                   ExecuteCommandLists( DX12Render *render );
static bool                   FlushCommandQueue( DX12Render *render );
static D3D12_RESOURCE_BARRIER GetBackbufferSwapTransition( BackbufferSwapResult requested, DX12Render *render );
static bool                   GetWindowExtent( HWND window, UINT *width, UINT *height );
static void                   Reset( DX12Render *render );
static void                   SetDefaultViewport( DX12Render *render );


/*******************************************************************
*
*   Render_Init()
*
*   DESCRIPTION:
*       Initialize the graphics system.
*       Returns TRUE if the DirectX 12 library was successfully
*       loaded.
*
*******************************************************************/

bool Render_Init( Universe *universe )
{    
SingletonRenderComponent* component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
component->ptr = malloc( sizeof( DX12Render ) );
if( !component->ptr )
    {
    return( false );
    }

DX12Render *render = (DX12Render*)component->ptr;
*render = {};
render->msaa_count           = 1;
render->msaa_quality         = 0;
render->render_target_format = DXGI_FORMAT_R8G8B8A8_UNORM;
render->depth_stencil_format = DXGI_FORMAT_D24_UNORM_S8_UINT;

/* obtain handle to main application window */
render->hwnd = ::GetActiveWindow();
if( render->hwnd == NULL )
    {
    goto failure;
    }
    
/* get the native dimensions of the application window */
if( !GetWindowExtent( render->hwnd, &render->window_width, &render->window_height ) )
    {
    return( false );
    }

/* device and graphics interface factory */
if( !CreateDevice( render ) )
    {
    goto failure;
    }

/* descriptor sizes */
for( uint32_t i = 0; i < cnt_of_array( render->descriptor_sizes ); i++ )
    {
    render->descriptor_sizes[ i ] = render->device->GetDescriptorHandleIncrementSize( (D3D12_DESCRIPTOR_HEAP_TYPE)i );
    }

/* fence */
if( FAILED( render->device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &render->fence ) ) ) )
    {
    goto failure;
    }

/* command buffer, command lists, and allocator */
if( !CreateCommandObjects( render ) )
    {
    goto failure;
    }

/* swap chain */
if( !CreateSwapChain( render ) )
    {
    goto failure;
    }

/* desciptor heaps */
if( !CreateDescriptorHeaps( render ) )
    {
    goto failure;
    }

/* backbuffer views */
if( !CreateRenderTargetViews( render ) )
    {
    goto failure;
    }

/* depth/stencil */
if( !CreateDepthStencil( render ) )
    {
    goto failure;
    }

/* set the viewport */
SetDefaultViewport( render );

if( FAILED( render->gfx->Close() ) )
    {
    goto failure;
    }

ExecuteCommandLists( render );
if( !FlushCommandQueue( render ) )
    {
    goto failure;
    }

return( true );

failure:
    {
    assert( false );
    Reset( render );
    return( false );
    }

} /* Render_Init() */


/*******************************************************************
*
*   Render_Destroy()
*
*   DESCRIPTION:
*       Destroy the Render system and free its resources.
*
*******************************************************************/

void Render_Destroy( Universe* universe )
{
DX12Render *render = AsDX12Render( universe );

Reset( render );

free( render );
render = NULL;

} /* Render_Destroy() */


/*******************************************************************
*
*   Render_DoFrame()
*
*   DESCRIPTION:
*       Advance the system one frame.
*
*******************************************************************/

void Render_DoFrame( float frame_delta, Universe *universe )
{
DX12Render *render = AsDX12Render( universe );

if( FAILED( render->command_allocator->Reset() ) )
    {
    assert( false );
    return;
    }

if( FAILED( render->gfx->Reset( render->command_allocator, NULL ) ) )
    {
    assert( false );
    return;
    }


D3D12_RESOURCE_BARRIER to_backbuffer = GetBackbufferSwapTransition( BACKBUFFER_SWAP_RESULT_BACKBUFFER, render );
render->gfx->ResourceBarrier( 1, &to_backbuffer );

SetDefaultViewport( render );
ClearBackbuffer( Utilities_MakeColor4f( 0.0f, 1.0f, 0.0f, 1.0f ), render );
ClearDepthStencil( 1.0f, 0, render );

D3D12_CPU_DESCRIPTOR_HANDLE backbuffer_view    = GetCurrentBackbufferView( render );
D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_view = GetDepthStencilView( render );
render->gfx->OMSetRenderTargets( 1, &backbuffer_view, TRUE, &depth_stencil_view );

D3D12_RESOURCE_BARRIER to_presentation = GetBackbufferSwapTransition( BACKBUFFER_SWAP_RESULT_PRESENTATION, render );
render->gfx->ResourceBarrier( 1, &to_presentation );

if( FAILED( render->gfx->Close() ) )
    {
    assert( false );
    return;
    }

ExecuteCommandLists( render );

/* flip the backbuffer */
if( FAILED( render->swap_chain->Present( 0, 0 ) ) )
    {
    assert( false );
    return;
    }

render->backbuffer_current = ( render->backbuffer_current + 1 ) % SWAP_CHAIN_DOUBLE_BUFFER;

if( !FlushCommandQueue( render ) )
    {
    assert( false );
    return;
    }

} /* Render_DoFrame() */


/*******************************************************************
*
*   AsDX12Render()
*
*   DESCRIPTION:
*       Get the singleton system state.
*
*******************************************************************/

static DX12Render * AsDX12Render( Universe* universe )
{
SingletonRenderComponent * component = (SingletonRenderComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
return( (DX12Render *)component->ptr );

} /* AsDX12Render() */


/*******************************************************************
*
*   ClearBackbuffer()
*
*   DESCRIPTION:
*       Clear the backbuffer to the given color.
*
*******************************************************************/

static void ClearBackbuffer( const Color4f clear_color, DX12Render *render )
{
render->gfx->ClearRenderTargetView( GetCurrentBackbufferView( render ), clear_color.f, 0, NULL );

} /* ClearBackbuffer() */


/*******************************************************************
*
*   ClearDepthStencil()
*
*   DESCRIPTION:
*       Clear the depth/stencil buffer to the given depth and
*       stencil values.
*
*******************************************************************/

static void ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, DX12Render *render )
{
#define DEPTH_AND_STENCIL           ( D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL )

render->gfx->ClearDepthStencilView( GetDepthStencilView( render ), DEPTH_AND_STENCIL, clear_depth, clear_stencil, 0, NULL );

#undef DEPTH_AND_STENCIL
} /* ClearDepthStencil() */


/*******************************************************************
*
*   CreateCommandObjects()
*
*   DESCRIPTION:
*       Create the objects needed to record and submit render
*       commands.
*
*******************************************************************/

static bool CreateCommandObjects( DX12Render *render )
{
/* command queue */
D3D12_COMMAND_QUEUE_DESC desc = {};
desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
if( FAILED( render->device->CreateCommandQueue( &desc, IID_PPV_ARGS( &render->command_queue ) ) ) )
    {
    return( false );
    }

/* allocator */
if( FAILED( render->device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &render->command_allocator ) ) ) )
    {
    return( false );
    }

/* command recording list */
if( FAILED( render->device->CreateCommandList( NODE_MASK_SINGLE_GPU, D3D12_COMMAND_LIST_TYPE_DIRECT, render->command_allocator, NULL, IID_PPV_ARGS( &render->gfx ) ) ) )
    {
    return( false );
    }

return( true );

} /* CreateCommandObjects() */


/*******************************************************************
*
*   CreateDepthStencil()
*
*   DESCRIPTION:
*       Create render target views for each backbuffer.
*
*******************************************************************/

static bool CreateDepthStencil( DX12Render *render )
{
/* stencil buffer */
D3D12_HEAP_PROPERTIES heap_props = {};
heap_props.Type                 = D3D12_HEAP_TYPE_DEFAULT;
heap_props.CPUPageProperty      = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
heap_props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
heap_props.CreationNodeMask     = NODE_MASK_SINGLE_GPU;
heap_props.VisibleNodeMask      = NODE_MASK_SINGLE_GPU;

D3D12_RESOURCE_DESC desc = {};
desc.Dimension          = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
desc.Format             = render->depth_stencil_format;
desc.Width              = render->window_width;
desc.Height             = render->window_height;
desc.MipLevels          = 1;
desc.DepthOrArraySize   = 1;
desc.Flags              = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
desc.SampleDesc.Count   = render->msaa_count;
desc.SampleDesc.Quality = render->msaa_quality;
desc.Layout             = D3D12_TEXTURE_LAYOUT_UNKNOWN;
desc.Alignment          = 0;

D3D12_CLEAR_VALUE clear_value = {};
clear_value.Format               = render->depth_stencil_format;
clear_value.DepthStencil.Depth   = 1.0f;
clear_value.DepthStencil.Stencil = 0;

if( FAILED( render->device->CreateCommittedResource( &heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, &clear_value, IID_PPV_ARGS( &render->depth_stencil_buffer ) ) ) )
    {
    return( false );
    }

/* depth/stencil view */
render->device->CreateDepthStencilView( render->depth_stencil_buffer, NULL, GetDepthStencilView( render ) );

/* transition the texture for depth/stencil writes */
D3D12_RESOURCE_BARRIER barrier = {};
barrier.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
barrier.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
barrier.Transition.pResource   = render->depth_stencil_buffer;
barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
barrier.Transition.StateAfter  = D3D12_RESOURCE_STATE_DEPTH_WRITE;
barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

render->gfx->ResourceBarrier( 1, &barrier );

return( true );

} /* CreateDepthStencil() */


/*******************************************************************
*
*   CreateDescriptorHeaps()
*
*   DESCRIPTION:
*       Create the render target and depth/stencil descriptor heaps.
*
*******************************************************************/

static bool CreateDescriptorHeaps( DX12Render *render )
{
D3D12_DESCRIPTOR_HEAP_DESC desc;

/* render target */
desc = {};
desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
desc.NumDescriptors = SWAP_CHAIN_DOUBLE_BUFFER;
desc.NodeMask       = NODE_MASK_SINGLE_GPU;
desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
if( FAILED( render->device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &render->render_target_heap ) ) ) )
    {
    return( false );
    }

/* depth stencil */
desc = {};
desc.Type           = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
desc.NumDescriptors = 1;
desc.NodeMask       = NODE_MASK_SINGLE_GPU;
desc.Flags          = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
if( FAILED( render->device->CreateDescriptorHeap( &desc, IID_PPV_ARGS( &render->depth_stencil_heap ) ) ) )
    {
    return( false );
    }

return( true );

} /* CreateDescriptorHeaps() */


/*******************************************************************
*
*   CreateDevice()
*
*   DESCRIPTION:
*       Create the DirectX device.
*
*******************************************************************/

static bool CreateDevice( DX12Render *render )
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

if( FAILED( CreateDXGIFactory2( debug_flags, IID_PPV_ARGS( &render->dxgi_factory ) ) ) )
    {
    return( false );
    }

if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &render->device ) ) ) )
    {
    IDXGIAdapter       *warp;
    if( FAILED( render->dxgi_factory->EnumWarpAdapter( IID_PPV_ARGS( &warp ) ) ) )
        {
        ComSafeRelease( &warp );
        return( false );
        }

    if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &render->device ) ) ) )
        {
        ComSafeRelease( &warp );
        return( false );
        }
    }
      
return( true );

} /* CreateDevice() */


/*******************************************************************
*
*   CreateRenderTargetViews()
*
*   DESCRIPTION:
*       Create render target views for each backbuffer.
*
*******************************************************************/

static bool CreateRenderTargetViews( DX12Render *render )
{
D3D12_CPU_DESCRIPTOR_HANDLE heap_location = render->render_target_heap->GetCPUDescriptorHandleForHeapStart();
for( uint32_t i = 0; i < cnt_of_array( render->backbuffers ); i++ )
    {
    if( FAILED( render->swap_chain->GetBuffer( (UINT)i, IID_PPV_ARGS( &render->backbuffers[ i ] ) ) ) )
        {
        return( false );
        }

    render->device->CreateRenderTargetView( render->backbuffers[ i ], NULL, heap_location );
    heap_location.ptr += render->descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];
    }

return( true );

} /* CreateRenderTargetViews() */


/*******************************************************************
*
*   CreateSwapChain()
*
*   DESCRIPTION:
*       Create the DirectX device.
*
*******************************************************************/

static bool CreateSwapChain( DX12Render *render )
{
/* fill out the descriptions */
DXGI_SWAP_CHAIN_DESC  desc    = {};
DXGI_MODE_DESC       *pmode   = &desc.BufferDesc;
DXGI_SAMPLE_DESC     *psample = &desc.SampleDesc;

desc.BufferCount  = SWAP_CHAIN_DOUBLE_BUFFER;
desc.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
desc.Flags        = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
desc.OutputWindow = render->hwnd;
desc.SwapEffect   = DXGI_SWAP_EFFECT_FLIP_DISCARD;
desc.Windowed     = TRUE;

pmode->Width                   = render->window_width;
pmode->Height                  = render->window_height;
pmode->RefreshRate.Numerator   = 60;
pmode->RefreshRate.Denominator = 1;
pmode->Scaling                 = DXGI_MODE_SCALING_UNSPECIFIED;
pmode->ScanlineOrdering        = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
pmode->Format                  = render->render_target_format;

psample->Count   = render->msaa_count;
psample->Quality = render->msaa_quality;;

if( FAILED( render->dxgi_factory->CreateSwapChain( render->command_queue, &desc, &render->swap_chain ) ) )
    {
    return( false );
    }
      
return( true );

} /* CreateSwapChain() */


/*******************************************************************
*
*   ExecuteCommandLists()
*
*   DESCRIPTION:
*       Execute all the commands recorded on our command lists.
*
*******************************************************************/

static void ExecuteCommandLists( DX12Render *render )
{
ID3D12CommandList *command_list = render->gfx;
render->command_queue->ExecuteCommandLists( 1, &command_list );

} /* ExecuteCommandLists() */


/*******************************************************************
*
*   FlushCommandQueue()
*
*   DESCRIPTION:
*       Block until all the graphics commands in the queue have
*       been completed.
*
*******************************************************************/

static bool FlushCommandQueue( DX12Render *render )
{
if( FAILED( render->command_queue->Signal( render->fence, ++render->fence_current ) ) )
    {
    return( false );
    }

if( render->fence->GetCompletedValue() < render->fence_current )
    {
    HANDLE blocking = ::CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
    if( FAILED( render->fence->SetEventOnCompletion( render->fence_current, blocking ) ) )
        {
        return( false );
        }

    ::WaitForSingleObject( blocking, INFINITE );
    ::CloseHandle( blocking );
    }

return( true );

} /* FlushCommandQueue() */
    

/*******************************************************************
*
*   GetBackbufferSwapTransition()
*
*   DESCRIPTION:
*       Get the barrier that transitions the backbuffer to flip.
*
*******************************************************************/

static D3D12_RESOURCE_BARRIER GetBackbufferSwapTransition( BackbufferSwapResult requested, DX12Render *render )
{
D3D12_RESOURCE_BARRIER ret = {};
ret.Type                   = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
ret.Flags                  = D3D12_RESOURCE_BARRIER_FLAG_NONE;
ret.Transition.pResource   = GetCurrentBackbuffer( render );
ret.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

if( requested == BACKBUFFER_SWAP_RESULT_BACKBUFFER )
    {
    ret.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    ret.Transition.StateAfter  = D3D12_RESOURCE_STATE_RENDER_TARGET;
    }
else
    {
    ret.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    ret.Transition.StateAfter  = D3D12_RESOURCE_STATE_PRESENT;
    }

return( ret );

} /* GetBackbufferSwapTransition() */


/*******************************************************************
*
*   GetWindowExtent()
*
*   DESCRIPTION:
*       Get the width and height of the given window.
*
*******************************************************************/

static bool GetWindowExtent( HWND window, UINT *width, UINT *height )
{
RECT rect;
if( !GetWindowRect( window, &rect ) )
    {
    return( false );
    }

*width  = (UINT)( rect.right  - rect.left );
*height = (UINT)( rect.bottom - rect.top  );

return( true );

} /* GetWindowExtent() */


/*******************************************************************
*
*   Reset()
*
*   DESCRIPTION:
*       Release all the renderer resources.
*
*******************************************************************/

static void Reset( DX12Render *render )
{
ComSafeRelease( &render->depth_stencil_buffer );
for( uint32_t i = 0; i < cnt_of_array( render->backbuffers ); i++ )
    {
    ComSafeRelease( &render->backbuffers[ i ] );
    }

ComSafeRelease( &render->depth_stencil_heap );
ComSafeRelease( &render->render_target_heap );
ComSafeRelease( &render->swap_chain );
ComSafeRelease( &render->gfx );
ComSafeRelease( &render->command_queue );
ComSafeRelease( &render->command_allocator );
ComSafeRelease( &render->fence );
ComSafeRelease( &render->device );
ComSafeRelease( &render->dxgi_factory );

*render = {};

} /* Reset() */


/*******************************************************************
*
*   SetDefaultViewport()
*
*   DESCRIPTION:
*       Create the DirectX device.
*
*******************************************************************/

static void SetDefaultViewport( DX12Render *render )
{
D3D12_VIEWPORT viewport = {};
viewport.Width    = (float)render->window_width;
viewport.Height   = (float)render->window_height;
viewport.TopLeftX = 0.0f;
viewport.TopLeftY = 0.0f;
viewport.MinDepth = 0.0f;
viewport.MaxDepth = 1.0f;

render->gfx->RSSetViewports( 1, &viewport );

} /* SetDefaultViewport() */
