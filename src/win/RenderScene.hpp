#pragma once

#include "Math.hpp"
#include "RenderModels.hpp"

#define RENDER_SCENE_SCENE_OBJECT_MAX_COUNT \
                                    ( 100 )

namespace RenderScene
{

typedef struct _SceneObject
    {
    RenderModels::ModelMesh
                       *mesh;
    Float4x4            world_xfm;
    } SceneObject;

typedef enum _ScenePassName
    {
    SCENE_PASS_NAME_FORWARD_OPAQUE,
    /* count */
    SCENE_PASS_NAME_COUNT
    } ScenePassName;

typedef struct _ScenePass
    {
    int i;
    SceneObject        *objects[ RENDER_SCENE_SCENE_OBJECT_MAX_COUNT ];
    } ScenePass;

typedef struct _Scene
    {
    uint32_t            object_count;
    SceneObject         objects[ RENDER_SCENE_SCENE_OBJECT_MAX_COUNT ];
    ScenePass           passes[ SCENE_PASS_NAME_COUNT ];
    } Scene;

} /* namespace RenderScene */