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
#include "RenderPipelines.hpp"
#include "RenderScene.hpp"
#include "RenderShaders.hpp"
#include "ResourceLoader.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"


#define RENDER_TARGET_VIEW_COUNT    ( RenderPass::DEFAULT_PASS_RTV_COUNT )

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

static inline Texture * GetCurrentBackbuffer( Engine *engine )
{
return( &engine->surfaces.backbuffers[ engine->surfaces.backbuffer_current ] );

} /* GetCurrentBackbuffer() */


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
//static bool     InitPasses( Engine *engine );
static bool     InitPipelines( Engine *engine );
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
*   DescriptorHeap_AllocateBatch()
*
*   DESCRIPTION:
*       Allocate a number of descriptor handles from the given heap.
*
*******************************************************************/

DescriptorHandle RenderEngine::DescriptorHeap_Allocate( DescriptorHeap *heap )
{
DescriptorHandle ret = {};
DescriptorHeap_AllocateBatch( 1, 1, &ret, heap );

return( ret );

} /* DescriptorHeap_Allocate() */


/*******************************************************************
*
*   DescriptorHeap_AllocateBatch()
*
*   DESCRIPTION:
*       Allocate a number of descriptor handles from the given heap.
*
*******************************************************************/

bool RenderEngine::DescriptorHeap_AllocateBatch( const uint32_t allocate_cnt, uint32_t out_capacity, DescriptorHandle *out, DescriptorHeap *heap )
{
uint32_t remaining = heap->capacity - heap->head;
if( remaining < allocate_cnt )
    {
    if( heap->is_ring )
        {
        /* Ring Allocator */
        if( heap->frames_since_wrap < RENDER_ENGINE_FRAME_COUNT )
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

} /* DescriptorHeap_AllocateBatch() */


/*******************************************************************
*
*   DescriptorHeap_Create()
*
*   DESCRIPTION:
*       Create a descriptor heap by allocating it within the given
*       device.
*
*******************************************************************/

bool RenderEngine::DescriptorHeap_Create( const D3D12_DESCRIPTOR_HEAP_TYPE type, const bool is_ring, const uint32_t capacity, ID3D12Device *device, DescriptorHeap *heap )
{
*heap = {};
heap->capacity          = capacity;
heap->type              = type;
heap->is_ring           = is_ring;
heap->is_shader_visible = ( type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV
                         || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER );
heap->descriptor_size   = device->GetDescriptorHandleIncrementSize( heap->type );

D3D12_DESCRIPTOR_HEAP_DESC desc = RenderInitializers::GetHeapDescriptor( capacity, heap->is_shader_visible, type );
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

} /* DescriptorHeap_Create() */


/*******************************************************************
*
*   DescriptorHeap_Destroy()
*
*   DESCRIPTION:
*       Destroy the given descriptor heap.
*
*******************************************************************/

void RenderEngine::DescriptorHeap_Destroy( DescriptorHeap *heap )
{
if( heap->heap )
    {
    heap->heap->Release();
    }

*heap = {};

} /* DescriptorHeap_Destroy() */


/*******************************************************************
*
*   DescriptorHeap_Reset()
*
*   DESCRIPTION:
*       Reset all allocations of the given descriptor heap.
*
*******************************************************************/

void RenderEngine::DescriptorHeap_Reset( DescriptorHeap *heap )
{
heap->head = 0;

} /* DescriptorHeap_Reset() */


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
engine->commands.gfx->ClearDepthStencilView( engine->surfaces.depth_stencil.handle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, clear_depth, clear_stencil, 0, NULL );

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

for( uint32_t i = 0; i < cnt_of_array( engine->surfaces.backbuffers ); i++ )
    {
    ComSafeRelease( &engine->surfaces.backbuffers[ i ] );
    }

ComSafeRelease( &engine->surfaces.depth_stencil );

engine->window.width  = width;
engine->window.height = height;

//RenderPass::Default_OnResize( &engine->passes.default_pass );

DescriptorHeap_Reset( &engine->surfaces.rtv_heap );
if( FAILED( engine->surfaces.swap_chain->ResizeBuffers( cnt_of_array( engine->surfaces.backbuffers ), width, height, RENDER_TARGET_FORMAT, DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH ) )
 || !CreateRenderTargetViews( engine ) )
    {
    debug_assert_always();
    return;
    }

DescriptorHeap_Reset( &engine->surfaces.dsv_heap );
CreateDepthStencil( engine );
SetViewport( Math_Float2Make( 0.0f, 0.0f ), Math_Float2Make( 1.0f, 1.0f ), engine );

if( FAILED( engine->commands.gfx->Close() ) )
    {
    assert( false );
    return;
    }

ExecuteCommandLists( engine );
FlushCommandQueue( engine );

engine->surfaces.backbuffer_current = 0;

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
    debug_assert_always();
    return;
    }

ClearBackbuffer( Utilities_MakeColor4f( 0.3f, 0.3f, 1.0f, 1.0f ), engine );
DrawScenes( engine, universe );

if( !EndFrame( engine ) )
    {
    debug_assert_always();
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

ModelComponent * Render_LoadModel( const char *asset_name, const EntityId entity, Universe *universe )
{
(void)Render_FreeModel( entity, universe );
ModelComponent *instance = (ModelComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_MODEL, universe );
instance->asset_name = AssetFile_CopyNameString( asset_name );

/* TODO <MPA> TESTING REMOVE */
Engine *engine = (Engine*)AsRenderEngine( universe );
RenderShaders::ShaderModule *shader = RenderShaders::ShaderCache_GetShader( "shader_vs_mvp", &engine->pipelines.shader_cache );
/* TODO <MPA> TESTING REMOVE */

return( instance );

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

D3D12_CPU_DESCRIPTOR_HANDLE backbuffer = GetCurrentBackbuffer( engine )->handle;
engine->commands.gfx->OMSetRenderTargets( 1, &backbuffer, TRUE, &engine->surfaces.depth_stencil.handle );

D3D12_RESOURCE_BARRIER as_backbuffer = RenderInitializers::GetRenderTargetTransition( D3D12_RESOURCE_STATE_RENDER_TARGET, GetCurrentBackbuffer( engine )->resource );
engine->commands.gfx->ResourceBarrier( 1, &as_backbuffer );

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
engine->commands.gfx->ClearRenderTargetView( GetCurrentBackbuffer( engine )->handle, clear_color.f, 0, NULL );

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
D3D12_HEAP_PROPERTIES heap_props = RenderInitializers::GetDefaultHeapProperties();
D3D12_RESOURCE_DESC desc = RenderInitializers::GetDepthStencilResourceDescriptor( engine->window.width, engine->window.height );

D3D12_CLEAR_VALUE clear_value = {};
clear_value.Format               = desc.Format;
clear_value.DepthStencil.Depth   = FAR_DEPTH_VALUE;
clear_value.DepthStencil.Stencil = 0;

engine->surfaces.depth_stencil.width  = (uint16_t)engine->window.width;
engine->surfaces.depth_stencil.height = (uint16_t)engine->window.height;
engine->surfaces.depth_stencil.handle = DescriptorHeap_Allocate( &engine->surfaces.dsv_heap ).cpu_hndl;
if( FAILED( engine->device.ptr->CreateCommittedResource( &heap_props, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, &clear_value, IID_PPV_ARGS( &engine->surfaces.depth_stencil.resource ) ) ) )
    {
    return( false );
    }

/* depth/stencil view */
engine->device.ptr->CreateDepthStencilView( engine->surfaces.depth_stencil.resource, NULL, engine->surfaces.depth_stencil.handle );

/* transition the texture for depth/stencil writes */
D3D12_RESOURCE_BARRIER barrier = RenderInitializers::GetResourceTransition( D3D12_RESOURCE_STATE_COMMON, D3D12_RESOURCE_STATE_DEPTH_WRITE, engine->surfaces.depth_stencil.resource );
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

return( DescriptorHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_RTV, DESCRIPTOR_HEAP_IS_LINEAR, rtv_count, engine->device.ptr, &engine->surfaces.rtv_heap )
     && DescriptorHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_DSV, DESCRIPTOR_HEAP_IS_LINEAR, dsv_count, engine->device.ptr, &engine->surfaces.dsv_heap ) );

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
for( uint32_t i = 0; i < cnt_of_array( engine->surfaces.backbuffers ); i++ )
    {
    Texture *backbuffer = &engine->surfaces.backbuffers[ i ];
    backbuffer->width  = engine->window.width;
    backbuffer->height = engine->window.height;
    backbuffer->handle = DescriptorHeap_Allocate( &engine->surfaces.rtv_heap ).cpu_hndl;
    if( FAILED( engine->surfaces.swap_chain->GetBuffer( (UINT)i, IID_PPV_ARGS( &backbuffer->resource ) ) ) )
        {
        return( false );
        }

    engine->device.ptr->CreateRenderTargetView( backbuffer->resource, NULL, backbuffer->handle );
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
if( FAILED( engine->device.dxgi->CreateSwapChain( engine->commands.queue, &desc, &engine->surfaces.swap_chain ) ) )
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
HashMap *map = &engine->scene_draw.map.map;
SceneComponent **draw_order = engine->scene_draw.draw_order;

HashMap_Clear( map );

NonOwningGroup_CreateIterator( universe, &engine->group, group_ids( COMPONENT_SCENE ) );
SceneComponent *seen_scene;
clr_array( draw_order );
while( NonOwningGroup_GetNext( &engine->group, NULL, (void**)&seen_scene ) )
    {
    debug_assert( HashMap_At( seen_scene->scene_name_hash, map ) == NULL );
    
    RenderScene::Scene *render_scene = (RenderScene::Scene*)seen_scene->render_state;
    RenderScene::Scene_BeginFrame( render_scene );

    hard_assert( HashMap_Insert( seen_scene->scene_name_hash, &render_scene, map ) != NULL );
    draw_order[ map->size - 1 ] = seen_scene;
    }

if( map->size == 0 )
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

    RenderScene::Scene *home_scene = *(RenderScene::Scene**)HashMap_At( model->scene_name_hash, map ); // TODO <MPA> - Do we want an entity to be able to belong to more than one scene?
    Scene_RegisterObject( model->asset_name.str, model, &mtx_world, home_scene );
    }

/* do a quick and dirty bubble sort on the scenes to prioritize them by draw order */
for( uint32_t i = 0; i < map->size; i++ )
    {
    for( uint32_t j = 0; j < map->size - i - 1; j++ )
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

for( uint32_t i = 0; i < map->size; i++ )
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
Texture *backbuffer = GetCurrentBackbuffer( engine );

D3D12_RESOURCE_BARRIER as_presentation = RenderInitializers::GetRenderTargetTransition( D3D12_RESOURCE_STATE_PRESENT, backbuffer->resource );
engine->commands.gfx->ResourceBarrier( 1, &as_presentation );

if( FAILED( engine->commands.gfx->Close() ) )
    {
    return( false );
    }

ExecuteCommandLists( engine );
if( FAILED( engine->surfaces.swap_chain->Present( 0, 0 ) ) )
    {
    return( false );
    }

engine->surfaces.backbuffer_current = ( engine->surfaces.backbuffer_current + 1 ) % SWAP_CHAIN_DOUBLE_BUFFER;
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
HashMap_InitImplementation( &engine->scene_draw.map );

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

if( !InitPipelines( engine ) )
    {
    goto failure;
    }

//if( !InitPasses( engine ) )
//    {
//    goto failure;
//    }

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


///*******************************************************************
//*
//*   InitPasses()
//*
//*   DESCRIPTION:
//*       Initialize the passes.
//*
//*******************************************************************/
//
//static bool InitPasses( Engine *engine )
//{
//uint16_t rtv_count = 0;
//uint16_t srv_count = 0;
//
///* Default pass */
//DXGI_FORMAT default_format = DXGI_FORMAT_R32G32B32A32_UINT;
//if( !RenderPass::Default_Init( GetRenderTargetView( rtv_count, engine ), RenderPipelines::Pipelines_GetShaderResourceView( srv_count, &engine->pipelines ), default_format, engine, &engine->passes.default_pass ) )
//    {
//    return( false );
//    }
//
//rtv_count += RenderPass::DEFAULT_PASS_RTV_COUNT;
//srv_count += RenderPass::DEFAULT_PASS_SRV_COUNT;
//
//return( true );
//
//} /* InitPasses() */


/*******************************************************************
*
*   InitPipelines()
*
*   DESCRIPTION:
*       Initialize the render pipelines.
*
*******************************************************************/

static bool InitPipelines( Engine *engine )
{
return( RenderPipelines::Pipelines_Init( engine->device.ptr, &engine->pipelines ) );

} /* InitPipelines() */


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
memset( render_scene, 0, sizeof(*render_scene) );
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
RenderPipelines::Pipelines_Destroy( &engine->pipelines );
for( uint32_t i = 0; i < cnt_of_array( engine->frames ); i++ )
    {
    Frame *frame = &engine->frames[ i ];
    SweepFrameTrash( frame );
    ComSafeRelease( &frame->pass_default.per_object.cbuffer );
    ComSafeRelease( &frame->pass_default.per_pass.cbuffer );
    ComSafeRelease( &frame->command_allocator );
    }

RenderPass::Default_Destroy( &engine->passes.default_pass );

ComSafeRelease( &engine->surfaces.depth_stencil );
for( uint32_t i = 0; i < cnt_of_array( engine->surfaces.backbuffers ); i++ )
    {
    ComSafeRelease( &engine->surfaces.backbuffers[ i ] );
    }

RenderPipelines::Pipelines_Destroy( &engine->pipelines );

ComSafeRelease( &engine->surfaces.dsv_heap );
ComSafeRelease( &engine->surfaces.rtv_heap );
ComSafeRelease( &engine->surfaces.swap_chain );
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
Float2 window_s   = Math_Float2Make( (float)engine->window.width, (float)engine->window.height );
Float2 top_left_s = Math_Float2HadamardProduct( top_left, window_s );
Float2 extent_s   = Math_Float2HadamardProduct( extent,   window_s );

D3D12_VIEWPORT viewport = {};
viewport.TopLeftX = top_left_s.v.x;
viewport.TopLeftY = top_left_s.v.y;
viewport.Width    = extent_s.v.x;
viewport.Height   = extent_s.v.y;
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
