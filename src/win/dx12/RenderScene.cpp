#include <cmath>
#include <cstring>
#include <d3d12.h>

#include "ComUtilities.hpp"
#include "HashMap.hpp"
#include "Math.hpp"
#include "RenderEngine.hpp"
#include "RenderInitializers.hpp"
#include "RenderModels.hpp"
#include "RenderPipelines.hpp"
#include "RenderScene.hpp"
#include "Utilities.hpp"


#define MODEL_CACHE_SZ              ( 800 * 1024 * 1024 )
#define RTV_HEAP_CAPACITY           RENDER_SCENE_FULLSCREEN_RENDER_TARGET_COUNT
#define DEFAULT_PASS_RT_FORMAT      DXGI_FORMAT_R8G8B8A8_UNORM

using namespace Render;
using namespace RenderModels;

namespace Render { namespace Scene
{

static void CleanupFrame( Scene *scene );
static void RegisterModelObject( const uint32_t starting_hash, const bool is_fresh_load, const Model *model, const Float4x4 *xfm_world, Scene *scene );
static void ResizeSceneSurfaces( Scene *scene );


/*******************************************************************
*
*   Scene_BeginFrame()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

void Scene_BeginFrame( const Float2 viewport, Scene *scene )
{
Float2 viewport_window = Math_Float2HadamardProduct( Math_Float2Make( (float)scene->engine->window.width, (float)scene->engine->window.height ), viewport );
if( fabs( viewport_window.v.x - scene->viewport_window.v.x ) > 0.5
 || fabs( viewport_window.v.y - scene->viewport_window.v.y ) > 0.5 )
    {
    scene->viewport_window = viewport_window;
    ResizeSceneSurfaces( scene );
    }

scene->passes[ SCENE_PASS_NAME_FORWARD_OPAQUE ].object_count = 0;
scene->passes[ SCENE_PASS_NAME_FORWARD_TRANSPARENT ].object_count = 0;
for( uint32_t i = 0; i < scene->objects.map.size; i++ )
    {
    SceneObject *object = scene->object_refs[ i ];
    if( !object->seen_this_frame )
        {
        continue;
        }

    //object->model->meshes[ object->mesh_index ].
    
    }

} /* Scene_BeginFrame() */


/*******************************************************************
*
*   Scene_Destroy()
*
*   DESCRIPTION:
*       Free the scene's resources.
*
*******************************************************************/

void Scene_Destroy( Scene *scene )
{
for( uint32_t i = 0; i < cnt_of_array( scene->passes ); i++ )
    {
    ComSafeRelease( &scene->passes[ i ].builder.root_signature );
    Engine_TossTrash( &scene->passes[ i ].pso, scene->engine );
    }

for( uint32_t i = 0; i < cnt_of_array( scene->rt_fullscreen ); i++ )
    {
    Engine_TossTrash( &scene->rt_fullscreen[ i ].resource, scene->engine );
    }

ModelCache_Destroy( &scene->models );
DescriptorHeap_Destroy( scene->engine, &scene->rtv_heap );

} /* Scene_Destroy() */


/*******************************************************************
*
*   Scene_Draw()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

void Scene_Draw( Scene *scene )
{
Engine_ClearDepthStencil( Initializers::FAR_DEPTH_VALUE, 0, scene->engine );



CleanupFrame( scene );

} /* Scene_Draw() */


/*******************************************************************
*
*   Scene_Init()
*
*   DESCRIPTION:
*       Initialize the scene render state.
*
*******************************************************************/

void Scene_Init( Engine::Engine *engine, Scene *scene )
{
memset( scene, 0, sizeof( *scene ) );

scene->engine = engine;

HashMap_InitImplementation( &scene->objects );
DescriptorHeap_Create( D3D12_DESCRIPTOR_HEAP_TYPE_RTV, Engine::DESCRIPTOR_HEAP_IS_LINEAR, RTV_HEAP_CAPACITY, engine->device.ptr, &scene->rtv_heap );
RenderModels::ModelCache_Init( MODEL_CACHE_SZ, &scene->models );

/* Forward opaque pass */
    {
    ScenePass *pass = &scene->passes[ SCENE_PASS_NAME_FORWARD_OPAQUE ];
    pass->name = SCENE_PASS_NAME_FORWARD_OPAQUE;
    pass->builder = *Pipelines::Pipelines_GetBuilder( Pipelines::BUILDER_NAME_DEFAULT, &engine->pipelines );
    pass->builder.root_signature->AddRef();
    pass->builder.num_render_targets = 1;
    pass->builder.rt_formats[ 0 ]    = DEFAULT_PASS_RT_FORMAT;

    pass->pso = Pipelines::PipelineBuilder_BuildPipeline( &pass->builder, scene->engine->device.ptr );
    NameD3D( pass->pso, L"Scene::FowardOpaquePass::PipelineStateObject" );
    }

/* Forward transparent pass */
    {
    ScenePass *pass = &scene->passes[ SCENE_PASS_NAME_FORWARD_TRANSPARENT ];
    pass->name = SCENE_PASS_NAME_FORWARD_TRANSPARENT;
    pass->builder = *Pipelines::Pipelines_GetBuilder( Pipelines::BUILDER_NAME_DEFAULT, &engine->pipelines );
    pass->builder.root_signature->AddRef();
    pass->builder.num_render_targets = 1;
    pass->builder.rt_formats[ 0 ]    = DEFAULT_PASS_RT_FORMAT; pass->builder.blending.RenderTarget->BlendEnable = TRUE;
    pass->builder.depth_stencil = Initializers::GetDepthStencilDescriptor( Initializers::ENABLE_DEPTH_TEST, Initializers::DISABLE_DEPTH_WRITE );

    pass->pso = Pipelines::PipelineBuilder_BuildPipeline( &pass->builder, scene->engine->device.ptr );
    NameD3D( pass->pso, L"Scene::FowardTransparentPass::PipelineStateObject" );
    }

} /* Scene_Init() */


/*******************************************************************
*
*   Scene_RegisterObject()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

void Scene_RegisterObject( const char *asset_name, const void *object, const Float4x4 *xfm_world, Scene *scene )
{
uint32_t starting_hash = Utilities_HashPointer( object );

bool new_model = false;
const Model *model = (Model*)ModelCache_GetModel( asset_name, &new_model, &scene->models );
if( model )
    {
    RegisterModelObject( starting_hash, new_model, model, xfm_world, scene );
    }

} /* Scene_RegisterObject() */


/*******************************************************************
*
*   CleanupFrame()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

static void CleanupFrame( Scene *scene )
{
for( int32_t i = (int32_t)scene->objects.map.size - 1; i >= 0; i-- )
    {
    SceneObject *object = scene->object_refs[ i ];
    if( object->seen_this_frame )
        {
        continue;
        }

    scene->object_refs[ i ] = scene->object_refs[ scene->objects.map.size - 1 ];
    hard_assert( HashMap_Delete( object->hash, &scene->objects.map ) );
    }

for( uint32_t i = 0; i < scene->objects.map.size; i++ )
    {
    scene->object_refs[ i ]->seen_this_frame = false;
    }

} /* CleanupFrame() */


/*******************************************************************
*
*   RegisterModelObject()
*
*   DESCRIPTION:
*       Handle the change of scene display size.
*
*******************************************************************/

static void RegisterModelObject( const uint32_t starting_hash, const bool is_fresh_load, const Model *model, const Float4x4 *xfm_world, Scene *scene )
{
/* meshes */
for( uint32_t i = 0; i < model->mesh_count; i++ )
    {
    // TODO <MPA> - Will need something a bit more clever once we have more than just mesh objects (e.g. lights, etc).
    uint32_t mesh_hash = Utilities_HashU32( starting_hash + i );
    SceneObject *so = (SceneObject*)HashMap_At( mesh_hash, &scene->objects.map );
    if( !so )
        {
        so = (SceneObject*)HashMap_Insert( mesh_hash, NULL, &scene->objects.map );
        so->hash        = mesh_hash;
        so->mesh_index  = i;
        so->model       = model;

        scene->object_refs[ scene->objects.map.size - 1 ] = so;
        }

    so->seen_this_frame = true;
    Math_Float4x4MultiplyByFloat4x4( xfm_world, &model->meshes[ i ].transform, &so->xfm_world );
    }

/* textures */
if( is_fresh_load )
    {
    for( uint32_t i = 0; i < model->mesh_count; i++ )
        {
        Material *material = &model->materials[ model->meshes[ i ].material_index ];
        for( uint32_t j = 0; j < cnt_of_array( material->textures ); j++ )
            {
            if( !material->textures )
                {
                continue;
                }

            }
        }
    }

} /* RegisterModelObject() */


/*******************************************************************
*
*   ResizeSceneSurfaces()
*
*   DESCRIPTION:
*       Handle the change of scene display size.
*
*******************************************************************/

static void ResizeSceneSurfaces( Scene *scene )
{
DescriptorHeap_Reset( &scene->rtv_heap );

uint16_t viewport_width  = (uint16_t)( scene->viewport_window.v.x + 0.5f );
uint16_t viewport_height = (uint16_t)( scene->viewport_window.v.y + 0.5f );

D3D12_RESOURCE_DESC fullscreen_desc = Initializers::GetTexture2DResourceDescriptor( viewport_width, viewport_height, Initializers::TEXTURE_USAGE_RENDER_TARGET, DEFAULT_PASS_RT_FORMAT );

for( uint32_t i = 0; i < cnt_of_array( scene->rt_fullscreen ); i++ )
    {
    Engine::Texture *texture = &scene->rt_fullscreen[ i ];

    Engine_TossTrash( &texture->resource, scene->engine );
    _hr( scene->engine->device.ptr->CreateCommittedResource( Initializers::GetDefaultHeapProperties(), D3D12_HEAP_FLAG_NONE, &fullscreen_desc, D3D12_RESOURCE_STATE_RENDER_TARGET, NULL, IID_PPV_ARGS( &texture->resource ) ) );
    
    texture->handle = DescriptorHeap_Allocate( &scene->rtv_heap ).cpu_hndl;
    D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = Initializers::GetRenderTargetViewDescriptor( DEFAULT_PASS_RT_FORMAT );
    scene->engine->device.ptr->CreateRenderTargetView( texture->resource, &rtv_desc, texture->handle );
    }

} /* ResizeSceneSurfaces() */


} }/* namespace Render::Scene */