#include "Command.hpp"
#include "Event.hpp"
#include "GameMode.hpp"
#include "PlayerInput.hpp"
#include "Render.hpp"
#include "ResourceManager.hpp" // TODO <MPA> - REMOVE
#include "Universe.hpp"
#include "Sound.hpp"

using namespace ECS;
using namespace Game;


static Universe the_universe;


/*******************************************************************
*
*   Engine_Init()
*
*   DESCRIPTION:
*       Initialize the game engine.
*
*******************************************************************/

bool Engine_Init()
{
Universe_Init( &the_universe );

if( !Command_Init( &the_universe ) )		 return( false );
if( !Event_Init( &the_universe ) )			 return( false );
if( !Render_Init( &the_universe ) )			 return( false );
if( !Sound_Init( &the_universe ) )           return( false );
if( !PlayerInput_Init( &the_universe ) )	 return( false );
if( !GameMode_Init( &the_universe ) )		 return( false );

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
GameMode_DoFrame( frame_delta, &the_universe );
PlayerInput_DoFrame( frame_delta, &the_universe );
Render_DoFrame( frame_delta, &the_universe );
Sound_Update( frame_delta, &the_universe );

// TODO - <MPA> TESTING REMOVE
static bool do_once = true;
if( do_once )
    {
    EntityId model_entity = Universe_CreateNewEntity( &the_universe );

    Render_LoadModel( "model_fmod_splash", model_entity, &the_universe );
    do_once = false;
    }

// END TODO

/* always last */
Event_DoFrame( frame_delta, &the_universe );
Command_DoFrame( frame_delta, &the_universe );

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