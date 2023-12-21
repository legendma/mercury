#pragma once

#include "HashMap.hpp"
#include "Math.hpp"
#include "RenderEngine.hpp"
#include "RenderModels.hpp"
#include "RenderPipelines.hpp"

#define RENDER_SCENE_SCENE_OBJECT_MAX_COUNT \
                                    ( 100 )
#define RENDER_SCENE_FULLSCREEN_RENDER_TARGET_COUNT \
                                    ( 2 )

#define RENDER_SCENE_MAX_TRASH_COUNT \
                                    ( 50 )

namespace RenderScene
{

typedef struct _SceneObject
    {
    const RenderModels::Model
                       *model;
    uint32_t            mesh_index;
    Float4x4            xfm_world;
    uint32_t            hash;
    boolean             seen_this_frame;
    } SceneObject;

HASH_MAP_IMPLEMENT( SceneObjectMap, RENDER_SCENE_SCENE_OBJECT_MAX_COUNT, SceneObject );

typedef enum _ScenePassName
    {
    SCENE_PASS_NAME_FORWARD_OPAQUE,
    SCENE_PASS_NAME_FORWARD_TRANSPARENT,
    /* count */
    SCENE_PASS_NAME_COUNT
    } ScenePassName;

typedef struct _ScenePass
    {
    ScenePassName       name;
    SceneObject        *objects[ RENDER_SCENE_SCENE_OBJECT_MAX_COUNT ];
    uint32_t            object_count;
    ID3D12PipelineState
                       *pso;
    RenderPipelines::PipelineBuilder
                        builder;
    } ScenePass;

typedef struct _Scene
    {
    SceneObject        *object_refs[ RENDER_SCENE_SCENE_OBJECT_MAX_COUNT ];
    SceneObjectMap      objects;
    ScenePass           passes[ SCENE_PASS_NAME_COUNT ];
    RenderModels::ModelCache
                        models;
    RenderEngine::Engine
                       *engine;
    Float2              viewport_window;
    RenderEngine::Texture
                        rt_fullscreen[ RENDER_SCENE_FULLSCREEN_RENDER_TARGET_COUNT ];
    uint8_t             rt_fullscreen_next;
    RenderEngine::DescriptorHeap
                        rtv_heap;
    } Scene;


void Scene_BeginFrame( const Float2 viewport, Scene *scene );
void Scene_Destroy( Scene *scene );
void Scene_Draw( Scene *scene );
void Scene_Init( RenderEngine::Engine *engine, Scene *scene );
void Scene_RegisterObject( const char *asset_name, const void *object, const Float4x4 *xfm_world, Scene *scene );

} /* namespace RenderScene */