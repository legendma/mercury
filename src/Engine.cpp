
#include "PlayerInput.hpp"
#include "universe.hpp"

using namespace ECS;


static Universe the_universe;

bool Engine_Init()
{
Universe_Init( &the_universe );
PlayerInput_Init( &the_universe );

return (true);
}


void Engine_DoFrame( float frame_delta )
{
PlayerInput_DoFrame( frame_delta, &the_universe );

}

bool Engine_CleanUp()
{
PlayerInput_Destroy( &the_universe );
Universe_Destroy( &the_universe );

return (true);
}