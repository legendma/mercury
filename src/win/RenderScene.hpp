#pragma once

#include "HashMap.hpp"
#include "Math.hpp"
#include "RenderModels.hpp"

#define RENDER_SCENE_SCENE_OBJECT_MAX_COUNT \
                                    ( 100 )
#define RENDER_SCENE_SHADER_RESOURCE_MAX_COUNT \
                                    ( 100 )

namespace RenderEngine { struct _Engine; }
namespace RenderScene
{

typedef struct _SceneObject
    {
    RenderModels::Model
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
    /* count */
    SCENE_PASS_NAME_COUNT
    } ScenePassName;

typedef struct _ScenePass
    {
    ScenePassName       name;
    SceneObject        *objects[ RENDER_SCENE_SCENE_OBJECT_MAX_COUNT ];
    } ScenePass;

HASH_MAP_IMPLEMENT( SceneSRVMap, RENDER_SCENE_SHADER_RESOURCE_MAX_COUNT, D3D12_SHADER_RESOURCE_VIEW_DESC );

typedef struct _Scene
    {
    SceneObject        *object_refs[ RENDER_SCENE_SCENE_OBJECT_MAX_COUNT ];
    SceneObjectMap      objects;
    SceneSRVMap         shader_resources;
    ScenePass           passes[ SCENE_PASS_NAME_COUNT ];
    RenderModels::ModelCache
                        models;
    RenderEngine::_Engine
                       *engine;
    } Scene;


void Scene_BeginFrame( Scene *scene );
void Scene_Draw( Scene *scene );
void Scene_Init( RenderEngine::_Engine *engine, Scene *scene );
void Scene_RegisterObject( const char *asset_name, const void *object, const Float4x4 *mtx_world, Scene *scene );

} /* namespace RenderScene */