#include "RenderEngine.hpp"
#include "RenderScene.hpp"
#include "Utilities.hpp"


#define MODEL_CACHE_SZ              ( 800 * 1024 * 1024 )

using namespace RenderEngine;
using namespace RenderModels;

namespace RenderScene
{

static void Cleanup( Scene *scene );


/*******************************************************************
*
*   Scene_BeginFrame()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

void Scene_BeginFrame( Scene *scene )
{


} /* Scene_BeginFrame() */


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
Engine_ClearDepthStencil( FAR_DEPTH_VALUE, 0, scene->engine );


Cleanup( scene );

} /* Scene_Draw() */


/*******************************************************************
*
*   Scene_Init()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

void Scene_Init( RenderEngine::_Engine *engine, Scene *scene )
{
scene->engine = engine;

RenderModels::ModelCache_Init( MODEL_CACHE_SZ, &scene->models );

} /* Scene_Init() */


/*******************************************************************
*
*   Scene_RegisterObject()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

void Scene_RegisterObject( const char *asset_name, const void *object, const Float4x4 *mtx_world, Scene *scene )
{
Model *model = (Model*)ModelCache_GetModel( asset_name, &scene->models );
uint32_t starting_hash = Utilities_HashPointer( object );

for( uint32_t i = 0; i < model->mesh_count; i++ )
    {
    // TODO <MPA> - Will need something a bit more clever once we have more than just mesh objects (e.g. lights, etc).
    uint32_t mesh_hash = Utilities_HashU32( starting_hash + i );
    SceneObject *so = (SceneObject*)HashMap_At( mesh_hash, &scene->objects.map );
    if( so )
        {
        so->seen_this_frame = true;
        so->xfm_world = *mtx_world;

        continue;
        }

    so = (SceneObject*)HashMap_Insert( mesh_hash, NULL, &scene->objects.map );
    so->hash            = mesh_hash;
    so->mesh_index      = i;
    so->model           = model;
    so->seen_this_frame = true;
    so->xfm_world       = *mtx_world;

    scene->object_refs[ scene->objects.map.size - 1 ] = so;
    }

} /* Scene_RegisterObject() */


/*******************************************************************
*
*   Cleanup()
*
*   DESCRIPTION:
*       
*
*******************************************************************/

static void Cleanup( Scene *scene )
{
for( uint32_t i = (uint32_t)scene->objects.map.size - 1; i < max_uint_value( i ); i-- )
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

} /* Cleanup() */


} /* namespace RenderScene */