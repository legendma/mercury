
#include "PlayerInput.hpp"
#include "Render.hpp"
#include "Universe.hpp"

using namespace ECS;


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
Render_Init( &the_universe );
PlayerInput_Init( &the_universe );

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
PlayerInput_DoFrame( frame_delta, &the_universe );
Render_DoFrame( frame_delta, &the_universe );

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
PlayerInput_Destroy( &the_universe );
Render_Destroy( &the_universe );
Universe_Destroy( &the_universe );

return( true );

} /* Engine_Destroy() */