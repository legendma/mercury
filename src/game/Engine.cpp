#include "Command.hpp"
#include "Engine.hpp"
#include "Event.hpp"
#include "GameMode.hpp"
#include "PlayerInput.hpp"
#include "Render.hpp"
#include "Universe.hpp"
#include "Sound.hpp"

//#include "D3D12Core.hpp"
//#include "D3D12Level.hpp"
//D3D12::Level::Level *test_level;

using namespace ECS;
using namespace Game;


static Universe the_universe;

static void OnFirstFrame();


/*******************************************************************
*
*   Engine_ChangeResolutions()
*
*   DESCRIPTION:
*       Change the display resolution of the game.
*
*******************************************************************/

void Engine_ChangeResolutions( const uint16_t width, const uint16_t height )
{
//Render_ChangeResolutions( width, height, &the_universe );

} /* Engine_ChangeResolutions() */


/*******************************************************************
*
*   Engine_Init()
*
*   DESCRIPTION:
*       Initialize the game engine.
*
*******************************************************************/

bool Engine_Init( VkSurfaceKHR surface, VkInstance vulkan )
{
Universe_Init( &the_universe );

if( !Command_Init( &the_universe ) )                 return( false );
if( !Event_Init( &the_universe ) )                   return( false );
if( !Render_Init( surface, vulkan, &the_universe ) ) return( false );
if( !Sound_Init( &the_universe ) )                   return( false );
if( !PlayerInput_Init( &the_universe ) )             return( false );
if( !GameMode_Init( &the_universe ) )                return( false );

return( true );

} /* Engine_Init() */


/*******************************************************************
*
*   Engine_DoFrame()
*
*   DESCRIPTION:
*       Advance the game engine by one frame.
*
*******************************************************************/

void Engine_DoFrame( float frame_delta )
{
static bool is_first_frame = true;
if( is_first_frame )
    {
    OnFirstFrame();
    }

GameMode_DoFrame( frame_delta, &the_universe );
PlayerInput_DoFrame( frame_delta, &the_universe );
Render_DoFrame( frame_delta, &the_universe );
Sound_Update( frame_delta, &the_universe );

/* always last */
Event_DoFrame( frame_delta, &the_universe );
Command_DoFrame( frame_delta, &the_universe );

is_first_frame = false;
} /* Engine_DoFrame() */


/*******************************************************************
*
*   Engine_Destroy()
*
*   DESCRIPTION:
*       Destroy the game engine and free its resources.
*
*******************************************************************/

bool Engine_Destroy()
{
GameMode_Destroy( &the_universe );
PlayerInput_Destroy( &the_universe );
Render_Destroy( &the_universe );
Sound_Destroy ( &the_universe );
Event_Destroy( &the_universe );
Command_Destroy( &the_universe );
Universe_Destroy( &the_universe );

return( true );

} /* Engine_Destroy() */


/*******************************************************************
*
*   OnFirstFrame()
*
*   DESCRIPTION:
*       Perform some operations on the first frame of the update.
*
*******************************************************************/

static void OnFirstFrame()
{
// TODO <MPA> - Testing grounds, remove later
    {
    EntityId scene_test_entity = Universe_CreateNewEntity( &the_universe );
    SceneComponent *scene = (SceneComponent*)Universe_AttachComponentToEntity( scene_test_entity, COMPONENT_SCENE, &the_universe );
    scene->viewport_top_left = Math_Float2Make( 0.0f, 0.0f );
    scene->viewport_extent   = Math_Float2Make( 1.0f, 1.0f );
    strcpy_s( scene->scene_name, cnt_of_array( scene->scene_name ), "test_scene" );
    scene->scene_name_hash = Utilities_HashString2( scene->scene_name );

    EntityId model_test_entity = Universe_CreateNewEntity( &the_universe );
    TransformComponent *transform = (TransformComponent*)Universe_AttachComponentToEntity( model_test_entity, COMPONENT_TRANSFORM, &the_universe );
    transform->position = Math_Float3Make( 0.0f, 0.0f, 0.0f );
    transform->rotation = QUATERNION_IDENTITY;
    transform->scale    = Math_Float3Make( 1.0f, 1.0f, 1.0f );

    //ModelComponent *model = Render_LoadModel( "model_fmod_splash", model_test_entity, &the_universe );
    //model->scene_name_hash = scene->scene_name_hash;
    }

//    {
//    test_level = (D3D12::Level::Level*)malloc( sizeof(D3D12::Level::Level) );
//    do_debug_assert( D3D12::Level::Init( "test_level", test_level ) );
//    D3D12::Level::LoadAsync( "test_level", test_level );
//
//    //free( level );
//    }
//// TODO <MPA> - Testing grounds, remove later

} /* OnFirstFrame() */