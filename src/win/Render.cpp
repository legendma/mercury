#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dxgi1_4.h>
#include <d3d12sdklayers.h>
#include <d3dcompiler.h>
#include <winbase.h>
#include <winuser.h>

#include "AssetFile.hpp"
#include "Command.hpp"
#include "ComUtilities.hpp"
#include "HashMap.hpp"
#include "NonOwningGroup.hpp"
#include "Render.hpp"
#include "RenderEngine.hpp"
#include "RenderInitializers.hpp"
#include "RenderScene.hpp"
#include "RenderShaders.hpp"
#include "ResourceManager.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"


#define SHADER_CACHE_SZ             ( 250 * 1024 * 1024 )
#define RENDER_TARGET_VIEW_COUNT    ( RenderPass::DEFAULT_PASS_RTV_COUNT )
#define CONSTANT_BUFFER_VIEW_COUNT  ( 0 )
#define SHADER_RESOURCE_VIEW_COUNT  ( RenderPass::DEFAULT_PASS_SRV_COUNT )

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
debug_assert( Engine_CurrentFrame( engine )->pass_default.per_object.mapped == NULL );
debug_assert( Engine_CurrentFrame( engine )->pass_default.per_pass.mapped == NULL );
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
return( engine->swap_chain.backbuffers[ engine->swap_chain.backbuffer_current ] );

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
D3D12_CPU_DESCRIPTOR_HANDLE ret = engine->device.rtv_heap->GetCPUDescriptorHandleForHeapStart();
ret.ptr += engine->swap_chain.backbuffer_current * engine->device.descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];

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
return( engine->device.dsv_heap->GetCPUDescriptorHandleForHeapStart() );

} /* GetDepthStencilView() */


/*******************************************************************
*
*   GetRenderTargetView()
*
*   DESCRIPTION:
*       Get the render target view for the given index.
*
*******************************************************************/

static inline D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView( const uint16_t rtv_index, Engine *engine )
{
debug_assert( rtv_index < RENDER_TARGET_VIEW_COUNT );
D3D12_CPU_DESCRIPTOR_HANDLE ret = engine->device.rtv_heap->GetCPUDescriptorHandleForHeapStart();
ret.ptr += ( SWAP_CHAIN_DOUBLE_BUFFER + rtv_index ) * engine->device.descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];

return( ret );

} /* GetRenderTargetView() */


/*******************************************************************
*
*   GetShaderResourceView()
*
*   DESCRIPTION:
*       Get the shader resource view for the given index.
*
*******************************************************************/

static inline D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView( const uint16_t srv_index, Engine *engine )
{
debug_assert( srv_index < SHADER_RESOURCE_VIEW_COUNT );
D3D12_CPU_DESCRIPTOR_HANDLE ret = engine->device.cbv_srv_heap->GetCPUDescriptorHandleForHeapStart();
ret.ptr += ( CONSTANT_BUFFER_VIEW_COUNT + srv_index ) * engine->device.descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV ];

return( ret );

} /* GetShaderResourceView() */


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
for( uint32_t i = 0; i < frame->trash_can_count; i++ )
    {
    ComSafeRelease( &frame->trash_can[ i ] );
    }

frame->trash_can_count = 0;

} /* SweepFrameTrash() */


static Engine * AsRenderEngine( Universe* universe );
static bool     BeginFrame( Engine *engine );
static void     ClearBackbuffer( const Color4f clear_color, Engine *engine );
static bool     CreateCommandObjects( Engine *engine );
static bool     CreateDefaultBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out );
static bool     CreateDepthStencil( Engine *engine );
static bool     CreateDescriptorHeaps( Engine *engine );
static bool     CreateDevice( Engine *engine );
static bool     CreateRenderTargetViews( Engine *engine );
static bool     CreateSwapChain( Engine *engine );
static bool     CreateUploadBuffer( const uint32_t buffer_size, ID3D12Device *device, ID3D12Resource **out );
static void     DrawScenes( Engine *engine, Universe *universe );
static bool     EndFrame( Engine *engine );
static void     ExecuteCommandLists( Engine *engine );
static bool     FlushCommandQueue( Engine *engine );
static bool     GetWindowExtent( HWND window, UINT *width, UINT *height );
static bool     InitDirectX( Engine *engine );
static bool     InitFrames( Engine *engine );
static bool     InitPasses( Engine *engine );
static UniverseComponentOnAttachProc
                OnSceneAttach;
static UniverseComponentOnAttachProc
                OnSceneRemove;
static void     Reset( Engine *engine );
static bool     ScheduleBufferUpload( const void *data, const uint32_t data_sz, ID3D12Device *device, ID3D12GraphicsCommandList *gfx, ID3D12Resource *upload, ID3D12Resource *gpu );
static void     SetViewport( const Float2 top_left, const Float2 extent, Engine *engine );
static bool     WaitForFrameToFinish( uint64_t frame_num, Engine *engine );


/*******************************************************************
*
*   Engine_ClearDepthStencil()
*
*   DESCRIPTION:
*       Clear the depth/stencil buffer to the given depth and
*       stencil values.
*
*******************************************************************/

void RenderEngine::Engine_ClearDepthStencil( const float clear_depth, uint8_t clear_stencil, Engine *engine )
{
engine->commands.gfx->ClearDepthStencilView( GetDepthStencilView( engine ), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear_depth, clear_stencil, 0, NULL );

} /* Engine_ClearDepthStencil() */


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
if( FAILED( engine->commands.gfx->Reset( engine->commands.spare_allocator, NULL ) ) )
    {
    debug_assert_always();
    return;
    }

for( uint32_t i = 0; i < cnt_of_array( engine->swap_chain.backbuffers ); i++ )
    {
    ComSafeRelease( &engine->swap_chain.backbuffers[ i ] );
    }

ComSafeRelease( &engine->swap_chain.depth_stencil );

engine->window.width  = width;
engine->window.height = height;

RenderPass::Default_OnResize( &engine->passes.default_pass );

if( FAILED( engine->swap_chain.ptr->ResizeBuffers( cnt_of_array( engine->swap_chain.backbuffers ), width, height, RENDER_TARGET_FORMAT, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) )
 || !CreateRenderTargetViews( engine ) )
    {
    debug_assert_always();
    return;
    }

CreateDepthStencil( engine );
SetViewport( Math_Float2Make( 0.0f, 0.0f ), Math_Float2Make( (float)engine->window.width, (float)engine->window.height ), engine );


if( FAILED( engine->commands.gfx->Close() ) )
    {
    assert( false );
    return;
    }

ExecuteCommandLists( engine );
FlushCommandQueue( engine );

engine->swap_chain.backbuffer_current = 0;

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

DrawScenes( engine, universe );

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

Universe_RegisterComponentLifetime( COMPONENT_SCENE, OnSceneAttach, OnSceneRemove, universe );

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

void Render_LoadModel( const char *asset_name, const EntityId entity, Universe *universe )
{
/* entity needs to have a transform component to own a model instance */
debug_assert( Universe_TryGetComponent( entity, COMPONENT_TRANSFORM, universe ) != NULL );

(void)Render_FreeModel( entity, universe );
ModelComponent *instance = (ModelComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_MODEL, universe );
instance->asset_name = AssetFile_CopyNameString( asset_name );

/* TODO <MPA> TESTING REMOVE */
Engine *engine = (Engine*)AsRenderEngine( universe );
RenderShaders::ShaderModule *shader = RenderShaders::ShaderCache_GetShader( "shader_vs_mvp", &engine->shaders );
/* TODO <MPA> TESTING REMOVE */

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

uint64_t last_completed_frame = engine->commands.fence->GetCompletedValue();
if( frame->frame_count != 0
 && last_completed_frame < frame->frame_count )
    {
    WaitForFrameToFinish( frame->frame_count, engine );
    }

SweepFrameTrash( frame );
if( FAILED( frame->command_allocator->Reset() ) )
    {
    return( false );
    }

if( FAILED( engine->commands.gfx->Reset( frame->command_allocator, NULL ) ) )
    {
    return( false );
    }

D3D12_RESOURCE_BARRIER as_backbuffer = RenderInitializers::GetRenderTargetTransition( D3D12_RESOURCE_STATE_RENDER_TARGET, GetCurrentBackbuffer( engine ) );
engine->commands.gfx->ResourceBarrier( 1, &as_backbuffer );
ClearBackbuffer( Utilities_MakeColor4f( 0.3f, 0.3f, 1.0f, 1.0f ), engine );

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
engine->commands.gfx->ClearRenderTargetView( GetCurrentBackbufferView( engine ), clear_color.f, 0, NULL );

} /* ClearBackbuffer() */


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
if( FAILED( engine->device.ptr->CreateCommandQueue( &desc, IID_PPV_ARGS( &engine->commands.queue ) ) ) )
    {
    return( false );
    }

/* spare allocator - we'll use to the issue 'important' commands which fall outside the realm of per-frame */
if( FAILED( engine->device.ptr->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &engine->commands.spare_allocator ) ) ) )
    {
    return( false );
    }

/* command recording list */
if( FAILED( engine->device.ptr->CreateCommandList( NODE_MASK_SINGLE_GPU, D3D12_COMMAND_LIST_TYPE_DIRECT, engine->commands.spare_allocator, NULL, IID_PPV_ARGS( &engine->commands.gfx ) ) ) )
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
D3D12_RESOURCE_DESC desc = RenderInitializers::GetDepthStencilResourceDescriptor( engine->window.width, engine->window.height );

D3D12_CLEAR_VALUE clear_value = {};
clear_value.Format               = desc.Format;
clear_value.DepthStencil.Depth   = FAR_DEPTH_VALUE;
clear_value.DepthStencil.Stencil = 0;

if( FAILED( engine->device.ptr->CreateCommittedResource( &heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, &clear_value, IID_PPV_ARGS( &engine->swap_chain.depth_stencil ) ) ) )
    {
    return( false );
    }

/* depth/stencil view */
engine->device.ptr->CreateDepthStencilView( engine->swap_chain.depth_stencil, NULL, GetDepthStencilView( engine ) );

/* transition the texture for depth/stencil writes */
D3D12_RESOURCE_BARRIER barrier = RenderInitializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE, engine->swap_chain.depth_stencil );
engine->commands.gfx->ResourceBarrier( 1, &barrier );

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
uint16_t rtv_count = SWAP_CHAIN_DOUBLE_BUFFER
                   + RENDER_TARGET_VIEW_COUNT;
uint16_t dsv_count = 1;
uint16_t cbv_srv_count = CONSTANT_BUFFER_VIEW_COUNT
                       + SHADER_RESOURCE_VIEW_COUNT;

/* render targets */
D3D12_DESCRIPTOR_HEAP_DESC rtv = RenderInitializers::GetHeapDescriptor( rtv_count, false, D3D12_DESCRIPTOR_HEAP_TYPE_RTV );
if( FAILED( engine->device.ptr->CreateDescriptorHeap( &rtv, IID_PPV_ARGS( &engine->device.rtv_heap ) ) ) )
    {
    return( false );
    }

/* depth stencil */
D3D12_DESCRIPTOR_HEAP_DESC dsv = RenderInitializers::GetHeapDescriptor( dsv_count, false, D3D12_DESCRIPTOR_HEAP_TYPE_DSV );
if( FAILED( engine->device.ptr->CreateDescriptorHeap( &dsv, IID_PPV_ARGS( &engine->device.dsv_heap ) ) ) )
    {
    return( false );
    }

/* constant buffers and shader resources */
D3D12_DESCRIPTOR_HEAP_DESC cbv_srv = RenderInitializers::GetHeapDescriptor( cbv_srv_count, false, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
if( FAILED( engine->device.ptr->CreateDescriptorHeap( &cbv_srv, IID_PPV_ARGS( &engine->device.cbv_srv_heap ) ) ) )
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

if( FAILED( CreateDXGIFactory2( debug_flags, IID_PPV_ARGS( &engine->device.dxgi ) ) ) )
    {
    return( false );
    }

if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &engine->device.ptr ) ) ) )
    {
    IDXGIAdapter       *warp;
    if( FAILED( engine->device.dxgi->EnumWarpAdapter( IID_PPV_ARGS( &warp ) ) ) )
        {
        ComSafeRelease( &warp );
        return( false );
        }

    if( FAILED( D3D12CreateDevice( NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS( &engine->device.ptr ) ) ) )
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
D3D12_CPU_DESCRIPTOR_HANDLE heap_location = engine->device.rtv_heap->GetCPUDescriptorHandleForHeapStart();
for( uint32_t i = 0; i < cnt_of_array( engine->swap_chain.backbuffers ); i++ )
    {
    if( FAILED( engine->swap_chain.ptr->GetBuffer( (UINT)i, IID_PPV_ARGS( &engine->swap_chain.backbuffers[ i ] ) ) ) )
        {
        return( false );
        }

    engine->device.ptr->CreateRenderTargetView( engine->swap_chain.backbuffers[ i ], NULL, heap_location );
    heap_location.ptr += engine->device.descriptor_sizes[ D3D12_DESCRIPTOR_HEAP_TYPE_RTV ];
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
DXGI_SWAP_CHAIN_DESC desc = RenderInitializers::GetSwapChainDescriptor( engine->window.width, engine->window.height, engine->window.handle );
if( FAILED( engine->device.dxgi->CreateSwapChain( engine->commands.queue, &desc, &engine->swap_chain.ptr ) ) )
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
*   DrawScenes()
*
*   DESCRIPTION:
*       Update and draw each scene.
*
*******************************************************************/

static void DrawScenes( Engine *engine, Universe *universe )
{
#define MAX_SCENE_CNT               ( 10 )

/* Update all the scenes with their objects */
HashMapKey              keys[ MAX_SCENE_CNT ];
RenderScene::Scene     *values[ MAX_SCENE_CNT ];
HashMap                 map;
SceneComponent         *draw_order[ MAX_SCENE_CNT ];

HashMap_Init( cnt_of_array( keys ), sizeof(*values), &map, keys, values );

NonOwningGroup_CreateIterator( universe, &engine->group, group_ids( COMPONENT_SCENE ) );
SceneComponent *seen_scene;
clr_array( draw_order );
while( NonOwningGroup_GetNext( &engine->group, NULL, (void**)&seen_scene ) )
    {
    debug_assert( HashMap_At( seen_scene->scene_name_hash, &map ) == NULL );
    
    RenderScene::Scene *render_scene = (RenderScene::Scene*)seen_scene->render_state;
    RenderScene::Scene_BeginFrame( render_scene );

    hard_assert( HashMap_Insert( seen_scene->scene_name_hash, &seen_scene, &map ) != NULL );
    draw_order[ map.size - 1 ] = seen_scene;
    }

if( map.size == 0 )
    {
    return;
    }

EntityId model_entity;
NonOwningGroup_CreateIterator( universe, &engine->group, group_ids( COMPONENT_MODEL, COMPONENT_TRANSFORM ) );
while( NonOwningGroup_GetNext( &engine->group, &model_entity, NULL ) )
    {
    ModelComponent     *model     = (ModelComponent*)NonOwningGroup_GetComponent( COMPONENT_MODEL, &engine->group );
    TransformComponent *transform = (TransformComponent*)NonOwningGroup_GetComponent( COMPONENT_TRANSFORM, &engine->group );

    Float4x4 mtx_world;
    Math_Float4x4TransformSpin( transform->position, transform->rotation, transform->scale, &mtx_world );

    RenderScene::Scene *home_scene = (RenderScene::Scene*)HashMap_At( model->scene_name_hash, &map ); // TODO <MPA> - Do we want an entity to be able to belong to more than one scene?
    Scene_RegisterObject( model->asset_name.str, model, &mtx_world, home_scene );
    }

/* do a quick and dirty bubble sort on the scenes to prioritize them by draw order */
for( uint32_t i = 0; i < map.size; i++ )
    {
    for( uint32_t j = 0; j < map.size - i - 1; j++ )
        {
        if( draw_order[ j ]->draw_order < draw_order[ j + 1 ]->draw_order )
            {
            continue;
            }

        SceneComponent *temp = draw_order[ j ];
        draw_order[ j ] = draw_order[ j + 1 ];
        draw_order[ j + 1 ] = temp;
        }
    }

for( uint32_t i = 0; i < map.size; i++ )
    {
    SetViewport( draw_order[ i ]->viewport_top_left, draw_order[ i ]->viewport_extent, engine );
    RenderScene::Scene_Draw( (RenderScene::Scene*)draw_order[ i ]->render_state );
    }

#undef MAX_SCENE_CNT
} /* DrawScenes() */


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

D3D12_CPU_DESCRIPTOR_HANDLE backbuffer_view    = GetCurrentBackbufferView( engine );
D3D12_CPU_DESCRIPTOR_HANDLE depth_stencil_view = GetDepthStencilView( engine );
engine->commands.gfx->OMSetRenderTargets( 1, &backbuffer_view, TRUE, &depth_stencil_view );

D3D12_RESOURCE_BARRIER as_presentation = RenderInitializers::GetRenderTargetTransition( D3D12_RESOURCE_STATE_PRESENT, GetCurrentBackbuffer( engine ) );
engine->commands.gfx->ResourceBarrier( 1, &as_presentation );

if( FAILED( engine->commands.gfx->Close() ) )
    {
    return( false );
    }

ExecuteCommandLists( engine );

/* flip the backbuffer (this is weird, apparently it automagically posts to the same GPU command buffer as a command queue) */
if( FAILED( engine->swap_chain.ptr->Present( 0, 0 ) ) )
    {
    return( false );
    }

engine->swap_chain.backbuffer_current = ( engine->swap_chain.backbuffer_current + 1 ) % SWAP_CHAIN_DOUBLE_BUFFER;
frame->frame_count = ++engine->commands.last_submitted_frame;

if( FAILED( engine->commands.queue->Signal( engine->commands.fence, engine->commands.last_submitted_frame ) ) )
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
ID3D12CommandList *command_list = engine->commands.gfx;
engine->commands.queue->ExecuteCommandLists( 1, &command_list );

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
return( WaitForFrameToFinish( engine->commands.last_submitted_frame, engine ) );

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
engine->window.handle = ::GetActiveWindow();
if( engine->window.handle == NULL )
    {
    return( false );
    }
    
/* get the native dimensions of the application window */
if( !GetWindowExtent( engine->window.handle, &engine->window.width, &engine->window.height ) )
    {
    return( false );
    }

/* device and graphics interface factory */
if( !CreateDevice( engine ) )
    {
    goto failure;
    }

/* descriptor sizes */
for( uint32_t i = 0; i < cnt_of_array( engine->device.descriptor_sizes ); i++ )
    {
    engine->device.descriptor_sizes[ i ] = engine->device.ptr->GetDescriptorHandleIncrementSize( (D3D12_DESCRIPTOR_HEAP_TYPE)i );
    }

/* fence */
if( FAILED( engine->device.ptr->CreateFence( 0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS( &engine->commands.fence ) ) ) )
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

if( !InitPasses( engine ) )
    {
    goto failure;
    }

/* set the viewport */
SetViewport( Math_Float2Make( 0.0f, 0.0f ), Math_Float2Make( (float)engine->window.width, (float)engine->window.height ), engine );

if( FAILED( engine->commands.gfx->Close() ) )
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
#define DEFAULT_PASS_INITIAL_PER_OBJECT_CNT \
                                    ( 20 )

for( uint8_t i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    Frame *frame = &engine->frames[ i ];
    frame->frame_index = i;

    /* Default pass */
    frame->pass_default.per_object.max_elements = DEFAULT_PASS_INITIAL_PER_OBJECT_CNT;
    uint32_t default_per_object_byte_size = CalculateConstantBufferSize( sizeof(*frame->pass_default.per_object.mapped) * frame->pass_default.per_object.max_elements );
    if( !CreateUploadBuffer( default_per_object_byte_size, engine->device.ptr, &frame->pass_default.per_object.cbuffer ) )
        {
        return( false );
        }

    uint32_t default_per_pass_byte_size = CalculateConstantBufferSize( sizeof(*frame->pass_default.per_pass.mapped) );
    if( !CreateUploadBuffer( default_per_pass_byte_size, engine->device.ptr, &frame->pass_default.per_pass.cbuffer ) )
        {
        return( false );
        }

    /* command allocator */
    if( FAILED( engine->device.ptr->CreateCommandAllocator( D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS( &frame->command_allocator ) ) ) )
        {
        return( false );
        }
    }

return( true );

#undef DEFAULT_PASS_INITIAL_PER_OBJECT_CNT
} /* InitFrames() */


/*******************************************************************
*
*   InitPasses()
*
*   DESCRIPTION:
*       Initialize the passes.
*
*******************************************************************/

static bool InitPasses( Engine *engine )
{
uint16_t rtv_count = 0;
uint16_t srv_count = 0;

/* Default pass */
DXGI_FORMAT default_format = DXGI_FORMAT_R32G32B32A32_UINT;
if( !RenderPass::Default_Init( GetRenderTargetView( rtv_count, engine ), GetShaderResourceView( srv_count, engine ), default_format, engine, &engine->passes.default_pass ) )
    {
    return( false );
    }

rtv_count += RenderPass::DEFAULT_PASS_RTV_COUNT;
srv_count += RenderPass::DEFAULT_PASS_SRV_COUNT;

return( true );

} /* InitPasses() */


/*******************************************************************
*
*   OnSceneAttach()
*
*   DESCRIPTION:
*       Handle a scene component being created, and create its
*       private render state.
*
*******************************************************************/

static void OnSceneAttach( const EntityId entity, const ComponentClass cls, void *component, Universe *universe )
{
debug_assert( cls == COMPONENT_SCENE );
SceneComponent *new_scene = (SceneComponent*)component;

RenderScene::Scene *render_scene = (RenderScene::Scene*)malloc( sizeof( RenderScene::Scene ) );
new_scene->render_state = render_scene;

RenderScene::Scene_Init( AsRenderEngine( universe ), render_scene );

} /* OnSceneAttach() */


/*******************************************************************
*
*   OnSceneRemove()
*
*   DESCRIPTION:
*       Handle a scene component being created, and destroy its
*       private render state.
*
*******************************************************************/

static void OnSceneRemove( const EntityId entity, const ComponentClass cls, void *component, Universe *universe )
{
debug_assert( cls == COMPONENT_SCENE );
SceneComponent *dying_scene = (SceneComponent*)component;

free( dying_scene->render_state );

} /* OnSceneRemove() */


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
FlushCommandQueue( engine );
for( uint32_t i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    Frame *frame = &engine->frames[ i ];
    SweepFrameTrash( frame );
    ComSafeRelease( &frame->pass_default.per_object.cbuffer );
    ComSafeRelease( &frame->pass_default.per_pass.cbuffer );
    ComSafeRelease( &frame->command_allocator );
    }

RenderPass::Default_Destroy( &engine->passes.default_pass );

ComSafeRelease( &engine->swap_chain.depth_stencil );
for( uint32_t i = 0; i < cnt_of_array( engine->swap_chain.backbuffers ); i++ )
    {
    ComSafeRelease( &engine->swap_chain.backbuffers[ i ] );
    }

ComSafeRelease( &engine->device.cbv_srv_heap );
ComSafeRelease( &engine->device.dsv_heap );
ComSafeRelease( &engine->device.rtv_heap );
ComSafeRelease( &engine->swap_chain.ptr );
ComSafeRelease( &engine->commands.fence );
ComSafeRelease( &engine->commands.gfx );
ComSafeRelease( &engine->commands.spare_allocator );
ComSafeRelease( &engine->commands.queue );
ComSafeRelease( &engine->device.ptr );
ComSafeRelease( &engine->device.dxgi );

clr_struct( engine );

} /* Reset() */


/*******************************************************************
*
*   SetViewport()
*
*   DESCRIPTION:
*       Set the given viewport.
*
*******************************************************************/

static void SetViewport( const Float2 top_left, const Float2 extent, Engine *engine )
{
D3D12_VIEWPORT viewport = {};
viewport.Width    = (float)engine->window.width;
viewport.Height   = (float)engine->window.height;
viewport.TopLeftX = 0.0f;
viewport.TopLeftY = 0.0f;
viewport.MinDepth = 0.0f;
viewport.MaxDepth = 1.0f;

engine->commands.gfx->RSSetViewports( 1, &viewport );

} /* SetViewport() */


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
 || FAILED( engine->commands.fence->SetEventOnCompletion( frame_num, blocking ) ) )
    {
    return( false );
    }

::WaitForSingleObject( blocking, INFINITE );
::CloseHandle( blocking );

return( true );

} /* WaitForFrameToFinish() */
