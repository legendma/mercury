#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dxgi1_4.h>
#include <d3d12sdklayers.h>
#include <d3dcompiler.h>
#include <winbase.h>
#include <winuser.h>

#include "ComUtilities.hpp"
#include "HashMap.hpp"
#include "Render.hpp"
#include "RenderEngine.hpp"
#include "RenderInitializers.hpp"
#include "RenderScene.hpp"
#include "RenderShaders.hpp"
#include "ResourceManager.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"


#define MODEL_CACHE_SZ              ( 800 * 1024 * 1024 )
#define SHADER_CACHE_SZ             ( 250 * 1024 * 1024 )

using namespace ECS;
using namespace RenderEngine;


/*******************************************************************
*
*   AdvanceFrame()
*
*   DESCRIPTION:
*       Advance the render engine one frame.
*
*******************************************************************/

static inline void AdvanceFrame( Engine *engine )
{
debug_assert( Engine_CurrentFrame( engine )->per_object_data.mapped == NULL );
debug_assert( Engine_CurrentFrame( engine )->per_pass_data.mapped.none == NULL );
++engine->current_frame %= cnt_of_array( engine->frames );

} /* AdvanceFrame() */


/*******************************************************************
*
*   CalculateConstantBufferSize()
*
*   DESCRIPTION:
*       The constant buffer must be a multiple of 256 bytes.
*
*******************************************************************/

static inline uint32_t CalculateConstantBufferSize( const uint32_t size )
{
return( ( size + 255 ) & 0xff );

} /* CalculateConstantBufferSize() */


/*******************************************************************
*
*   GetCurrentBackbuffer()
*
*   DESCRIPTION:
*       Get the current backbuffer texture.
*
*******************************************************************/

static inline ID3D12Resource * GetCurrentBackbuffer( Engine *engine )
{
return( engine->backbuffers[ engine->backbuffer_current ] );

} /* GetCurrentBackbuffer() */


/*******************************************************************
*
*   GetCurrentBackbufferView()
*
*   DESCRIPTION:
*       Get the render target view for the current backbuffer.
*
*******************************************************************/

static inline D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentBackbufferView( Engine *engine )
{
D3D12_CPU_DESCRIPTOR_HANDLE ret = engine->render_target_heap->GetCPUDescriptorHandleForHeapStart();
ret.ptr += engine->backbuffer_current * engine->descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];

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

static inline D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView( Engine *engine )
{
return( engine->depth_stencil_heap->GetCPUDescriptorHandleForHeapStart() );

} /* GetDepthStencilView() */


static Engine * AsRenderEngine( Universe* universe );
static bool     BeginFrame( Engine *engine );
static void     ClearBackbuffer( const Color4f clear_color, Engine *engine );
static void     ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, Engine *engine );
static bool     CreateCommandObjects( Engine *engine );
static bool     CreateDefaultBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out );
static bool     CreateDepthStencil( Engine *engine );
static bool     CreateDescriptorHeaps( Engine *engine );
static bool     CreateDevice( Engine *engine );
static bool     CreateRenderTargetViews( Engine *engine );
static bool     CreateSwapChain( Engine *engine );
static bool     CreateUploadBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out );
static bool     EndFrame( Engine *engine );
static void     ExecuteCommandLists( Engine *engine );
static bool     FlushCommandQueue( Engine *engine );
static bool     GetWindowExtent( HWND window, UINT *width, UINT *height );
static bool     InitDirectX( Engine *engine );
static bool     InitFrames( Engine *engine );
static void     Reset( Engine *engine );
static bool     ScheduleBufferUpload( const void *data, const uint32_t data_sz, ID3D12Device *device, ID3D12GraphicsCommandList *gfx, ID3D12Resource *upload, ID3D12Resource *gpu );
static void     SetDefaultViewport( Engine *engine );
static bool     WaitForFrameToFinish( uint64_t frame_num, Engine *engine );


/*******************************************************************
*
*   Engine_CurrentFrame()
*
*   DESCRIPTION:
*       Get the frame that is currently being built.
*
*******************************************************************/

Frame * RenderEngine::Engine_CurrentFrame( Engine *engine )
{
return( &engine->frames[ engine->current_frame ] );

} /* Engine_CurrentFrame() */


/*******************************************************************
*
*   Render_ChangeResolutions()
*
*   DESCRIPTION:
*       Change the graphics screen resolution.
*
*******************************************************************/

void Render_ChangeResolutions( const uint16_t width, const uint16_t height, Universe *universe )
{
Engine *engine = AsRenderEngine( universe );

FlushCommandQueue( engine );
if( FAILED( engine->gfx->Reset( engine->spare_command_allocator, NULL ) ) )
    {
    debug_assert_always();
    return;
    }

for( uint32_t i = 0; i < cnt_of_array( engine->backbuffers ); i++ )
    {
    ComSafeRelease( &engine->backbuffers[ i ] );
    }

ComSafeRelease( &engine->depth_stencil_buffer );

engine->window_width  = width;
engine->window_height = height;

if( FAILED( engine->swap_chain->ResizeBuffers( cnt_of_array( engine->backbuffers ), width, height, RENDER_TARGET_FORMAT, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) )
 || !CreateRenderTargetViews( engine ) )
    {
    debug_assert_always();
    return;
    }

CreateDepthStencil( engine );
SetDefaultViewport( engine );

if( FAILED( engine->gfx->Close() ) )
    {
    assert( false );
    return;
    }

ExecuteCommandLists( engine );
FlushCommandQueue( engine );

engine->backbuffer_current = 0;

} /* Render_ChangeResolutions() */


/*******************************************************************
*
*   Render_Destroy()
*
*   DESCRIPTION:
*       Destroy the Render engine and free its resources.
*
*******************************************************************/

void Render_Destroy( Universe* universe )
{
Engine *engine = AsRenderEngine( universe );

Reset( engine );

free( engine );
SingletonRenderComponent *component = (SingletonRenderComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
component->ptr = NULL;

} /* Render_Destroy() */


/*******************************************************************
*
*   Render_DoFrame()
*
*   DESCRIPTION:
*       Advance the engine one frame.
*
*******************************************************************/

void Render_DoFrame( float frame_delta, Universe *universe )
{
Engine *engine = AsRenderEngine( universe );
if( !BeginFrame( engine ) )
    {
    assert( false );
    return;
    }

D3D12_RESOURCE_BARRIER as_backbuffer = RenderInitializers::GetRenderTargetTransition( D3D12_RESOURCE_STATE_RENDER_TARGET, GetCurrentBackbuffer( engine ) );
engine->gfx->ResourceBarrier( 1, &as_backbuffer );

SetDefaultViewport( engine );
ClearBackbuffer( Utilities_MakeColor4f( 0.3f, 0.3f, 1.0f, 1.0f ), engine );
ClearDepthStencil( FAR_DEPTH_VALUE, 0, engine );

D3D12_CPU_DESCRIPTOR_HANDLE backbuffer_view    = GetCurrentBackbufferView( engine );
D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_view = GetDepthStencilView( engine );
engine->gfx->OMSetRenderTargets( 1, &backbuffer_view, TRUE, &depth_stencil_view );

D3D12_RESOURCE_BARRIER as_presentation = RenderInitializers::GetRenderTargetTransition( D3D12_RESOURCE_STATE_PRESENT, GetCurrentBackbuffer( engine ) );
engine->gfx->ResourceBarrier( 1, &as_presentation );

if( FAILED( engine->gfx->Close() ) )
    {
    assert( false );
    return;
    }

ExecuteCommandLists( engine );

if( !EndFrame( engine ) )
    {
    assert( false );
    return;
    }

} /* Render_DoFrame() */


/*******************************************************************
*
*   Render_FreeModel()
*
*   DESCRIPTION:
*       Free the model attached to the entity.
*       Returns TRUE if a model existed and was free.  FALSE
*       otherwise.
*
*******************************************************************/

bool Render_FreeModel( const EntityId entity, Universe *universe )
{
Engine *engine = (Engine *)AsRenderEngine( universe );
ModelComponent *existing = (ModelComponent*)Universe_TryGetComponent( entity, COMPONENT_MODEL, universe );
if( !existing )
    {
    return( false );
    }
    
//Model *model = (Model*)HashMap_At( existing->asset_id, &engine->models.map );
Universe_RemoveComponentFromEntity( entity, COMPONENT_MODEL, universe );
//if( !model )
//    {
//    /* model should have existed */
//    assert( false );
//    return( true );
//    }
//
//assert( model->ref_count );
//model->ref_count--;

return( true );

} /* Render_FreeModel() */


/*******************************************************************
*
*   Render_Init()
*
*   DESCRIPTION:
*       Initialize the graphics engine.
*       Returns TRUE if the DirectX 12 library was successfully
*       loaded.
*
*******************************************************************/

bool Render_Init( Universe *universe )
{    
SingletonRenderComponent* component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
component->ptr = malloc( sizeof( Engine ) );
if( !component->ptr )
    {
    return( false );
    }

Engine *engine = (Engine*)component->ptr;
clr_struct( engine );

if( !InitDirectX( engine ) )
    {
    return( false );
    }

RenderModels::ModelCache_Init( MODEL_CACHE_SZ, &engine->models );
RenderShaders::ShaderCache_Init( SHADER_CACHE_SZ, &engine->shaders );

return( true );

} /* Render_Init() */


/*******************************************************************
*
*   Render_LoadModel()
*
*   DESCRIPTION:
*       Load the given model by asset name.
*
*******************************************************************/

bool Render_LoadModel( const char *asset_name, const EntityId entity, Universe *universe )
{
Engine *engine = (Engine*)AsRenderEngine( universe );
/* TODO <MPA> TESTING REMOVE */
RenderShaders::ShaderModule *shader = RenderShaders::ShaderCache_GetShader( "shader_vs_mvp", &engine->shaders );

/* TODO <MPA> TESTING REMOVE */

(void)Render_FreeModel( entity, universe );

RenderModels::Model *model = RenderModels::ModelCache_GetModel( asset_name, &engine->models );
if( !model )
    {
    return( false );
    }

ModelComponent *instance = (ModelComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_MODEL, universe );
instance->asset_id = model->asset_id;

return( true );

} /* Render_LoadModel() */


/*******************************************************************
*
*   AsRenderEngine()
*
*   DESCRIPTION:
*       Get the singleton engine state.
*
*******************************************************************/

static Engine * AsRenderEngine( Universe* universe )
{
SingletonRenderComponent * component = (SingletonRenderComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_RENDER, universe );
return( (Engine*)component->ptr );

} /* AsRenderEngine() */


/*******************************************************************
*
*   BeginFrame()
*
*   DESCRIPTION:
*       Begin a new render frame.
*
*******************************************************************/

static bool BeginFrame( Engine *engine )
{
Frame *frame = Engine_CurrentFrame( engine );

uint64_t last_completed_frame = engine->fence->GetCompletedValue();
if( frame->frame_count != 0
 && last_completed_frame < frame->frame_count )
    {
    WaitForFrameToFinish( frame->frame_count, engine );
    }

if( FAILED( frame->command_allocator->Reset() ) )
    {
    return( false );
    }

if( FAILED( engine->gfx->Reset( frame->command_allocator, NULL ) ) )
    {
    return( false );
    }

return( true );

} /* BeginFrame() */


/*******************************************************************
*
*   ClearBackbuffer()
*
*   DESCRIPTION:
*       Clear the backbuffer to the given color.
*
*******************************************************************/

static void ClearBackbuffer( const Color4f clear_color, Engine *engine )
{
engine->gfx->ClearRenderTargetView( GetCurrentBackbufferView( engine ), clear_color.f, 0, NULL );

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

static void ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, Engine *engine )
{
engine->gfx->ClearDepthStencilView( GetDepthStencilView( engine ), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear_depth, clear_stencil, 0, NULL );

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

static bool CreateCommandObjects( Engine *engine )
{ 
/* command queue */
D3D12_COMMAND_QUEUE_DESC desc = {};
desc.Type  = D3D12_COMMAND_LIST_TYPE_DIRECT;
desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
if( FAILED( engine->device->CreateCommandQueue( &desc, IID_PPV_ARGS( &engine->command_queue ) ) ) )
    {
    return( false );
    }

/* spare allocator - we'll use to the issue 'important' commands which fall outside the realm of per-frame */
if( FAILED( engine->device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &engine->spare_command_allocator ) ) ) )
    {
    return( false );
    }

/* command recording list */
if( FAILED( engine->device->CreateCommandList( NODE_MASK_SINGLE_GPU, D3D12_COMMAND_LIST_TYPE_DIRECT, engine->spare_command_allocator, NULL, IID_PPV_ARGS( &engine->gfx ) ) ) )
    {
    return( false );
    }

return( true );

} /* CreateCommandObjects() */


/*******************************************************************
*
*   CreateDefaultBuffer()
*
*   DESCRIPTION:
*       Create a buffer in the default heap initialized to the
*       common state.
*
*******************************************************************/

static bool CreateDefaultBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out )
{
D3D12_HEAP_PROPERTIES props = RenderInitializers::GetDefaultHeapProperties();
D3D12_RESOURCE_DESC desc = RenderInitializers::GetBufferResourceDescriptor( buffer_size );

return( SUCCEEDED( device->CreateCommittedResource( &props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, NULL, IID_PPV_ARGS( out ) ) ) );

} /* CreateDefaultBuffer() */


/*******************************************************************
*
*   CreateDepthStencil()
*
*   DESCRIPTION:
*       Create render target views for each backbuffer.
*
*******************************************************************/

static bool CreateDepthStencil( Engine *engine )
{
/* stencil buffer */
D3D12_HEAP_PROPERTIES heap_props = RenderInitializers::GetDefaultHeapProperties();
D3D12_RESOURCE_DESC desc = RenderInitializers::GetDepthStencilResourceDescriptor( engine->window_width, engine->window_height );

D3D12_CLEAR_VALUE clear_value = {};
clear_value.Format               = desc.Format;
clear_value.DepthStencil.Depth   = FAR_DEPTH_VALUE;
clear_value.DepthStencil.Stencil = 0;

if( FAILED( engine->device->CreateCommittedResource( &heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, &clear_value, IID_PPV_ARGS( &engine->depth_stencil_buffer ) ) ) )
    {
    return( false );
    }

/* depth/stencil view */
engine->device->CreateDepthStencilView( engine->depth_stencil_buffer, NULL, GetDepthStencilView( engine ) );

/* transition the texture for depth/stencil writes */
D3D12_RESOURCE_BARRIER barrier = RenderInitializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE, engine->depth_stencil_buffer );
engine->gfx->ResourceBarrier( 1, &barrier );

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

static bool CreateDescriptorHeaps( Engine *engine )
{
/* swap chain */
D3D12_DESCRIPTOR_HEAP_DESC swap_chain = RenderInitializers::GetHeapDescriptor( 2, false, D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
if( FAILED( engine->device->CreateDescriptorHeap( &swap_chain, IID_PPV_ARGS( &engine->render_target_heap ) ) ) )
    {
    return( false );
    }

/* depth stencil */
D3D12_DESCRIPTOR_HEAP_DESC depth_stencil = RenderInitializers::GetHeapDescriptor( 1, false, D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
if( FAILED( engine->device->CreateDescriptorHeap( &depth_stencil, IID_PPV_ARGS( &engine->depth_stencil_heap ) ) ) )
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

static bool CreateDevice( Engine *engine )
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

if( FAILED( CreateDXGIFactory2( debug_flags, IID_PPV_ARGS( &engine->dxgi_factory ) ) ) )
    {
    return( false );
    }

if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &engine->device ) ) ) )
    {
    IDXGIAdapter       *warp;
    if( FAILED( engine->dxgi_factory->EnumWarpAdapter( IID_PPV_ARGS( &warp ) ) ) )
        {
        ComSafeRelease( &warp );
        return( false );
        }

    if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &engine->device ) ) ) )
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

static bool CreateRenderTargetViews( Engine *engine )
{
D3D12_CPU_DESCRIPTOR_HANDLE heap_location = engine->render_target_heap->GetCPUDescriptorHandleForHeapStart();
for( uint32_t i = 0; i < cnt_of_array( engine->backbuffers ); i++ )
    {
    if( FAILED( engine->swap_chain->GetBuffer( (UINT)i, IID_PPV_ARGS( &engine->backbuffers[ i ] ) ) ) )
        {
        return( false );
        }

    engine->device->CreateRenderTargetView( engine->backbuffers[ i ], NULL, heap_location );
    heap_location.ptr += engine->descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];
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

static bool CreateSwapChain( Engine *engine )
{
/* fill out the descriptions */
DXGI_SWAP_CHAIN_DESC desc = RenderInitializers::GetSwapChainDescriptor( engine->window_width, engine->window_height, engine->hwnd );
if( FAILED( engine->dxgi_factory->CreateSwapChain( engine->command_queue, &desc, &engine->swap_chain ) ) )
    {
    return( false );
    }
      
return( true );

} /* CreateSwapChain() */


/*******************************************************************
*
*   CreateUploadBuffer()
*
*   DESCRIPTION:
*       Create a upload buffer.
*
*******************************************************************/

static bool CreateUploadBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out )
{
D3D12_HEAP_PROPERTIES props = RenderInitializers::GetUploadHeapProperties();
D3D12_RESOURCE_DESC desc = RenderInitializers::GetBufferResourceDescriptor( buffer_size );

return( SUCCEEDED( device->CreateCommittedResource( &props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL, IID_PPV_ARGS( out ) ) ) );

} /* CreateUploadBuffer() */


/*******************************************************************
*
*   EndFrame()
*
*   DESCRIPTION:
*       End the current frame.
*
*******************************************************************/

static bool EndFrame( Engine *engine )
{
Frame *frame = Engine_CurrentFrame( engine );

/* flip the backbuffer (this is weird, apparently it automagically posts to the same GPU command buffer as a command queue) */
if( FAILED( engine->swap_chain->Present( 0, 0 ) ) )
    {
    return( false );
    }

engine->backbuffer_current = ( engine->backbuffer_current + 1 ) % SWAP_CHAIN_DOUBLE_BUFFER;
frame->frame_count = ++engine->last_submitted_frame;

if( FAILED( engine->command_queue->Signal( engine->fence, engine->last_submitted_frame ) ) )
    {
    return( false );
    }

AdvanceFrame( engine );

return( true );

} /* EndFrame() */


/*******************************************************************
*
*   ExecuteCommandLists()
*
*   DESCRIPTION:
*       Execute all the commands recorded on our command lists.
*
*******************************************************************/

static void ExecuteCommandLists( Engine *engine )
{
ID3D12CommandList *command_list = engine->gfx;
engine->command_queue->ExecuteCommandLists( 1, &command_list );

} /* ExecuteCommandLists() */


/*******************************************************************
*
*   FlushCommandQueue()
*
*   DESCRIPTION:
*       Wait for all unfinished GPU commands to complete.
*
*******************************************************************/

static bool FlushCommandQueue( Engine *engine )
{
return( WaitForFrameToFinish( engine->last_submitted_frame, engine ) );

} /* FlushCommandQueue() */


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
*   InitDirectX()
*
*   DESCRIPTION:
*       Initialize the DirectX driver.
*
*******************************************************************/

static bool InitDirectX( Engine *engine )
{
/* obtain handle to main application window */
engine->hwnd = ::GetActiveWindow();
if( engine->hwnd == NULL )
    {
    return( false );
    }
    
/* get the native dimensions of the application window */
if( !GetWindowExtent( engine->hwnd, &engine->window_width, &engine->window_height ) )
    {
    return( false );
    }

/* device and graphics interface factory */
if( !CreateDevice( engine ) )
    {
    goto failure;
    }

/* descriptor sizes */
for( uint32_t i = 0; i < cnt_of_array( engine->descriptor_sizes ); i++ )
    {
    engine->descriptor_sizes[ i ] = engine->device->GetDescriptorHandleIncrementSize( (D3D12_DESCRIPTOR_HEAP_TYPE)i );
    }

/* fence */
if( FAILED( engine->device->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &engine->fence ) ) ) )
    {
    goto failure;
    }

if( !InitFrames( engine ) )
    {
    goto failure;
    }

/* command buffer, command lists, and allocator */
if( !CreateCommandObjects( engine ) )
    {
    goto failure;
    }

/* swap chain */
if( !CreateSwapChain( engine ) )
    {
    goto failure;
    }

/* desciptor heaps */
if( !CreateDescriptorHeaps( engine ) )
    {
    goto failure;
    }

/* backbuffer views */
if( !CreateRenderTargetViews( engine ) )
    {
    goto failure;
    }

/* depth/stencil */
if( !CreateDepthStencil( engine ) )
    {
    goto failure;
    }

/* set the viewport */
SetDefaultViewport( engine );

if( FAILED( engine->gfx->Close() ) )
    {
    goto failure;
    }

ExecuteCommandLists( engine );
if( !FlushCommandQueue( engine ) )
    {
    goto failure;
    }

return( true );

failure:
    {
    assert( false );
    Reset( engine );
    return( false );
    }

} /* InitDirectX() */


/*******************************************************************
*
*   InitFrames()
*
*   DESCRIPTION:
*       Initialize the per-frame resources.
*
*******************************************************************/

static bool InitFrames( Engine *engine )
{
#define INITIAL_PER_OBJECT_ELEMENT_CNT \
                                    ( 20 )

for( uint8_t i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    Frame *frame = &engine->frames[ i ];

    /* per-object constant buffer */
    uint32_t element_cnt = INITIAL_PER_OBJECT_ELEMENT_CNT;
    uint32_t byte_size = CalculateConstantBufferSize( sizeof( GPUObjectData ) * element_cnt );

    if( !CreateUploadBuffer( byte_size, engine->device, &frame->per_object_data.elements ) )
        {
        return( false );
        }

    frame->per_object_data.max_elements = element_cnt;

    /* command allocator */
    if( FAILED( engine->device->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &frame->command_allocator ) ) ) )
        {
        return( false );
        }
    }

return( true );

#undef INITIAL_PER_OBJECT_ELEMENT_CNT
} /* InitFrames() */


/*******************************************************************
*
*   Reset()
*
*   DESCRIPTION:
*       Release all renderer resources.
*
*******************************************************************/

static void Reset( Engine *engine )
{
for( uint32_t i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    Frame *frame = &engine->frames[ i ];
    ComSafeRelease( &frame->per_object_data.elements );
    ComSafeRelease( &frame->command_allocator );
    }

ComSafeRelease( &engine->depth_stencil_buffer );
for( uint32_t i = 0; i < cnt_of_array( engine->backbuffers ); i++ )
    {
    ComSafeRelease( &engine->backbuffers[ i ] );
    }

ComSafeRelease( &engine->depth_stencil_heap );
ComSafeRelease( &engine->render_target_heap );
ComSafeRelease( &engine->swap_chain );
ComSafeRelease( &engine->fence );
ComSafeRelease( &engine->gfx );
ComSafeRelease( &engine->command_queue );
ComSafeRelease( &engine->device );
ComSafeRelease( &engine->dxgi_factory );

*engine = {0};

} /* Reset() */


/*******************************************************************
*
*   ScheduleBufferUpload()
*
*   DESCRIPTION:
*       Schedule a buffer to be uploaded to the GPU.  Assumes the
*       destination buffer is in the common state.
*
*******************************************************************/

static bool ScheduleBufferUpload( const void *data, const uint32_t data_sz, ID3D12Device *device, ID3D12GraphicsCommandList *gfx, ID3D12Resource *upload, ID3D12Resource *gpu )
{
/* Sanity check */
debug_if( true,
    ID3D12Device *dest_device = NULL;
    gpu->GetDevice( IID_PPV_ARGS( &dest_device ) );
    assert( device == dest_device );
    );

D3D12_RESOURCE_DESC gpu_desc    = gpu->GetDesc();
D3D12_RESOURCE_DESC upload_desc = upload->GetDesc();

D3D12_PLACED_SUBRESOURCE_FOOTPRINT layout;
unsigned int num_rows;
uint64_t row_sz;
uint64_t required_sz;
device->GetCopyableFootprints( &gpu_desc, 0, 1, 0, &layout, &num_rows, &row_sz, &required_sz );
if( upload_desc.Width < required_sz
 || upload_desc.Width < data_sz
 || !gpu )
    {
    return( false );
    }

void *uploading;
if( FAILED( upload->Map( 0, NULL, &uploading ) ) )
    {
    return( false );
    }

memcpy( uploading, data, data_sz );
upload->Unmap( 0, NULL );

D3D12_RESOURCE_BARRIER to_write = RenderInitializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_COPY_DEST, gpu );
gfx->ResourceBarrier( 1, &to_write );

gfx->CopyBufferRegion( gpu, 0, upload, 0, data_sz );

D3D12_RESOURCE_BARRIER to_read = RenderInitializers::GetResourceTransition( D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ, gpu );
gfx->ResourceBarrier( 1, &to_write );

return( true );

} /* ScheduleBufferUpload() */


/*******************************************************************
*
*   SetDefaultViewport()
*
*   DESCRIPTION:
*       Create the DirectX device.
*
*******************************************************************/

static void SetDefaultViewport( Engine *engine )
{
D3D12_VIEWPORT viewport = {};
viewport.Width    = (float)engine->window_width;
viewport.Height   = (float)engine->window_height;
viewport.TopLeftX = 0.0f;
viewport.TopLeftY = 0.0f;
viewport.MinDepth = 0.0f;
viewport.MaxDepth = 1.0f;

engine->gfx->RSSetViewports( 1, &viewport );

} /* SetDefaultViewport() */


/*******************************************************************
*
*   WaitForFrameToFinish()
*
*   DESCRIPTION:
*       Block until all the graphics commands in the queue have
*       been completed.
*
*******************************************************************/

static bool WaitForFrameToFinish( uint64_t frame_num, Engine *engine )
{
HANDLE blocking = ::CreateEventEx( NULL, NULL, 0, EVENT_ALL_ACCESS );
if( blocking == NULL
 || FAILED( engine->fence->SetEventOnCompletion( frame_num, blocking ) ) )
    {
    return( false );
    }

::WaitForSingleObject( blocking, INFINITE );
::CloseHandle( blocking );

return( true );

} /* WaitForFrameToFinish() */
