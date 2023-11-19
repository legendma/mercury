#include <fmod_studio.h>
#include <fmod_common.h>
#include <cstdio>
#include <cstdlib>

#include "universe.hpp"


using namespace ECS;

#define MAX_INSTANCES_OF_A_SOUND  ( 32 )

typedef struct _FMODSoundSystem
{
	FMOD_STUDIO_SYSTEM *fmod_system;

} FMODSoundSystem;

static FMODSoundSystem * AsFMODSoundSystem( Universe *universe );


/*******************************************************************
*
*   Sound_Init()
*
*   DESCRIPTION:
*	create and intialize the fmod sound system. 
*	returns true if suceeded, false if failed.
*
*******************************************************************/

bool Sound_Init( Universe *universe )
{
	SingletonSoundSystemComponent *component = (SingletonSoundSystemComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_SOUND_SYSTEM, universe );
	component->ptr = malloc( sizeof( _FMODSoundSystem ) );

	if( !component->ptr )
	{
		return(false);
	}

	FMODSoundSystem *system = (FMODSoundSystem *)component->ptr;
	*system = {};


FMOD_Studio_System_Create( &system->fmod_system, FMOD_VERSION );
FMOD_Studio_System_Initialize( system->fmod_system, MAX_INSTANCES_OF_A_SOUND, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, 0);

if( FMOD_Studio_System_IsValid(system->fmod_system ) == false )
{
printf(" ERROR: The FMOD sound system failed to initialize!");
return false;
} 

return true;
}/* Sound_Init() */


/*******************************************************************
*
*   Sound_Update()
*
*   DESCRIPTION:
*	Call this function to process all buffered FMOD API commands.
*	Note: This is nativly multi-threaded.
*
*
*******************************************************************/

void Sound_Update( float frame_delta, Universe *universe )
{
	FMODSoundSystem *system = AsFMODSoundSystem( universe );
	FMOD_Studio_System_Update( system->fmod_system);

}/* Sound_Update() */


/*******************************************************************
*
*   Sound_Destroy()
*
*   DESCRIPTION:
*	cleans up and destroys the sound system
*
*******************************************************************/

void Sound_Destroy( Universe *universe )
{
	FMODSoundSystem *system = AsFMODSoundSystem( universe );

	FMOD_Studio_System_Release( system->fmod_system);

	free( system );
	system = NULL;


}/* Sound_Destroy() */


/*******************************************************************
*
*   AsFMODSoundSystem()
*
*   DESCRIPTION:
*       Get the singleton sound system state.
*
*******************************************************************/

static FMODSoundSystem * AsFMODSoundSystem( Universe *universe )
{
	SingletonSoundSystemComponent *component = (SingletonSoundSystemComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_SOUND_SYSTEM, universe );
	return((FMODSoundSystem *)component->ptr);

} /* AsFMODSoundSystem() */



