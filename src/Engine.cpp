
#include "PlayerInput.hpp"




bool Engine_Init()
{

	/*  - setup controller input*/

	PlayerInput_InitializeController();

return (true);
}


void Engine_DoFrame(float frame_delta)
{

	PlayerInput_GetCurrentInput();

}

bool Engine_CleanUp()
{

	/*Shutdown controller*/
	PlayerInput_ShutdownController();


return (true);
}