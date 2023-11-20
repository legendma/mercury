#include <fmod_studio.h>
#include <fmod_common.h>
#include <fmod_errors.h>
#include <fmod.h>
#include <cstdio>
#include <cstdlib>

#include "Sound.hpp"
#include "Universe.hpp"
#include "Utilities.hpp"


using namespace ECS;

#define MAX_NUMBER_OF_SOUNDS          ( 512 )
#define MAX_INSTANCES_OF_A_SOUND      ( 512 )
#define MAX_NUMBER_OF_SOUND_BANKS     ( 32 )
#define MAX_NUMBER_OF_SOUND_CHANNELS  ( 32 )

#define MASTER_BANK_FILENAME            "Master.bank"			
#define MASTER_STRINGS_BANK_FILENAME    "Master.strings.bank"	
#define MASTER_ASSETS_BANK_FILENAME     "Master.assets.bank"	
#define MASTER_STREAMS_BANK_FILENAME    "Master.streams.bank"	

FMOD_RESULT fmod_error_code;

typedef struct _FMODSoundSystem
{
	FMOD_STUDIO_SYSTEM *fmod_studio_system;
	FMOD_SYSTEM        *fmod_system;
	FMOD_STUDIO_BANK   *master_bank;
	FMOD_STUDIO_BANK   *master_strings_bank;
	FMOD_STUDIO_BANK   *master_assets_bank;
	FMOD_STUDIO_BANK   *master_streams_bank;
	EntityId            test_sound_entity;
} FMODSoundSystem;


static FMODSoundSystem * AsFMODSoundSystem( Universe *universe );
static bool GetFMODSoundEvent( const char *sound_name, FMODSoundSystem *system, FMOD_STUDIO_EVENTDESCRIPTION **sound_event );
static bool LoadBank( const char *filename, FMOD_STUDIO_SYSTEM *fmod_studio_system, FMOD_STUDIO_LOAD_BANK_FLAGS load_flags, FMOD_STUDIO_BANK **output_bank );


/*******************************************************************
*
*   Sound_AssignSoundtToEntity()
*
*   DESCRIPTION:
*       Takes the sound event path string as input and assigns the FMOD
*		Event description to the componenet and creates an instance
*
*******************************************************************/

bool Sound_AssignSoundtToEntity( const char *sound_name, const bool overwrite_sound, const EntityId entity, Universe *universe )
{
	FMODSoundSystem *system = AsFMODSoundSystem( universe );

	/* attach the sound component to the entity or return the component if it already exists */
	SoundsComponent *component = (SoundsComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_SOUNDS, universe );
	
	/* If the entity already has a component and we don't want to overright it then exit*/
	if( component->sound_event_description != NULL
     && !overwrite_sound )
	{
	return( false );
	}
	
	/* add the sound to the component */
	GetFMODSoundEvent( sound_name, system, (FMOD_STUDIO_EVENTDESCRIPTION **)&component->sound_event_description );

	fmod_error_code = FMOD_Studio_EventDescription_CreateInstance( (FMOD_STUDIO_EVENTDESCRIPTION*)component->sound_event_description, (FMOD_STUDIO_EVENTINSTANCE **)&component->sound_event_instance );
	if( fmod_error_code != FMOD_OK )
	{
		printf( "ERROR: FMOD failed to create an instance of the sound. (%s) \n", FMOD_ErrorString( fmod_error_code ) );
		return false;
	}

	return true;
} /* Sound_AssignSoundtToEntity */


/*******************************************************************
*
*   Sound_Destroy()
*
*   DESCRIPTION:
*		Cleans up and destroys the sound system
*
*******************************************************************/

void Sound_Destroy( Universe *universe )
{
	FMODSoundSystem *system = AsFMODSoundSystem( universe );
	FMOD_Studio_System_FlushSampleLoading( system->fmod_studio_system );
	FMOD_Studio_System_FlushCommands( system->fmod_studio_system );
	FMOD_Studio_System_UnloadAll( system->fmod_studio_system );
	FMOD_Studio_System_Release( system->fmod_studio_system);

	free( system );
	system = NULL;

}/* Sound_Destroy() */


/*******************************************************************
*
*   Sound_Init()
*
*   DESCRIPTION:
*		create and intialize the fmod sound system. 
*		returns true if suceeded, false if failed.
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

	/* setup an Fmod studio system instance */
	fmod_error_code = FMOD_Studio_System_Create( &system->fmod_studio_system, FMOD_VERSION );
	if( fmod_error_code != FMOD_OK )
	{
	printf( "ERROR: The FMOD studio sound system failed to be created. (%s) \n", FMOD_ErrorString( fmod_error_code ) );
	return false;
	}

	/* get the core Fmod system */
	fmod_error_code = FMOD_Studio_System_GetCoreSystem( system->fmod_studio_system, &system->fmod_system );
	if( fmod_error_code != FMOD_OK )
	{
		printf( "ERROR: The FMOD studio sound system failed to be created. (%s) \n", FMOD_ErrorString( fmod_error_code ) );
		return false;
	}

	/* initialize the fmod studio system */
	fmod_error_code = FMOD_Studio_System_Initialize( system->fmod_studio_system, MAX_INSTANCES_OF_A_SOUND, FMOD_STUDIO_INIT_LIVEUPDATE, FMOD_INIT_NORMAL, NULL);
	if( fmod_error_code != FMOD_OK )
	{
		printf( "ERROR: The FMOD studio sound system failed to be initialized. (%s) \n", FMOD_ErrorString( fmod_error_code ) );
		return false;
	}

	/* load the master sound banks */
	if( !LoadBank( MASTER_BANK_FILENAME,		 system->fmod_studio_system, FMOD_STUDIO_LOAD_BANK_NORMAL, &system->master_bank			)
	 || !LoadBank( MASTER_STRINGS_BANK_FILENAME, system->fmod_studio_system, FMOD_STUDIO_LOAD_BANK_NORMAL, &system->master_strings_bank )
	 || !LoadBank( MASTER_ASSETS_BANK_FILENAME,	 system->fmod_studio_system, FMOD_STUDIO_LOAD_BANK_NORMAL, &system->master_assets_bank  )
	 || !LoadBank( MASTER_STREAMS_BANK_FILENAME, system->fmod_studio_system, FMOD_STUDIO_LOAD_BANK_NORMAL, &system->master_streams_bank ) )
		{
		return( false );
		}

	EntityId test_sound_entity = Universe_CreateNewEntity( universe );
	// TODO jw make a function that can load a bank using FMOD_Studio_System_LoadBankFile and load in the assets using FMOD_Studio_Bank_LoadSampleData. probably with some FMOD agnostic text handle to the bank.

return true;

}/* Sound_Init() */


/*******************************************************************
*
*   Sound_PlaySound()
*
*   DESCRIPTION:
*       This function will play one instance of the sound for the 
*		inputed entity.  if the sound is already playing or about to 
*		play then no additional sound will be triggered.
*
*******************************************************************/

bool Sound_PlaySound( const EntityId entity, Universe *universe )
{
	FMODSoundSystem *system = AsFMODSoundSystem( universe );
	SoundsComponent *component = (SoundsComponent *)Universe_TryGetComponent( entity, COMPONENT_SOUNDS, universe );
	if( component == NULL )
	{
		printf( " ERROR: This Entity does not have a sound component. Sound cannot be played" );
		return (false);
	}

	/* check if a sound is already playing or about to be played */
	FMOD_STUDIO_PLAYBACK_STATE sound_state = {};
	fmod_error_code = FMOD_Studio_EventInstance_GetPlaybackState( (FMOD_STUDIO_EVENTINSTANCE *)component->sound_event_instance, &sound_state );
	if( fmod_error_code != FMOD_OK )
	{
		printf( "ERROR: FMOD failed to check a sound instance playback state (%s %d) \n", FMOD_ErrorString( fmod_error_code ), sound_state );
		return (false);
	}

	/* check if the sound is already playing or about to play and don't Que up another sound. */
	if( sound_state != FMOD_STUDIO_PLAYBACK_STOPPED
	 && sound_state != FMOD_STUDIO_PLAYBACK_STOPPING )
	{
		return (true);
	}

	/* play the sound instance */
	fmod_error_code = FMOD_Studio_EventInstance_Start( (FMOD_STUDIO_EVENTINSTANCE*)component->sound_event_instance );
	if( fmod_error_code != FMOD_OK )
	{
		printf( "ERROR: FMOD failed to play sound (%s) \n", FMOD_ErrorString( fmod_error_code ) );
		return false;
	}

	return true;

}/* Sound_PlaySound() */


/*******************************************************************
*
*   Sound_StopSound()
*
*   DESCRIPTION:
*       This function stops playing all instances of the inputed sound
*
*******************************************************************/

bool Sound_StopSound( const char *sound_name, Universe *universe )
{
	FMODSoundSystem *system = AsFMODSoundSystem( universe );
	FMOD_STUDIO_EVENTDESCRIPTION *sound_event = NULL;
	GetFMODSoundEvent( sound_name, system, &sound_event );

	fmod_error_code = FMOD_Studio_EventDescription_ReleaseAllInstances( sound_event );
	if( fmod_error_code != FMOD_OK )
	{
		printf( "ERROR: FMOD failed to release all instances of the sound event. (%s) \n", FMOD_ErrorString( fmod_error_code ) );
		return false;
	}

	return true;

}/* Sound_StopSound() */


/*******************************************************************
*
*   Sound_Update()
*
*   DESCRIPTION:
*		Call this function to process all buffered FMOD API commands.
*		Note: This is nativly multi-threaded.
*
*******************************************************************/

void Sound_Update( float frame_delta, Universe *universe )
{
	FMODSoundSystem *system = AsFMODSoundSystem( universe );
	FMOD_Studio_System_Update( system->fmod_studio_system);

	/* dummy code for testing */
	Sound_AssignSoundtToEntity( "event:/test_sound", false, system->test_sound_entity, universe );

	SingletonControllerInputComponent *component = (SingletonControllerInputComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_CONTROLLER_INPUT, universe );
	if( component->is_pressed[ CONTROLLER_BUTTON_BOT_BUTTON ]( component ) )
	{
		Sound_PlaySound( system->test_sound_entity, universe);
	}

}/* Sound_Update() */


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


/*******************************************************************
*
*   GetFMODSoundEvent()
*
*   DESCRIPTION:
*       Get's the FMOD Event Description for the input sound
*
*******************************************************************/

static bool GetFMODSoundEvent( const char *sound_name, FMODSoundSystem *system, FMOD_STUDIO_EVENTDESCRIPTION **sound_event )
{
/* get the sound event handle for the input string */
	fmod_error_code = FMOD_Studio_System_GetEvent( system->fmod_studio_system, sound_name, sound_event );
	if( fmod_error_code != FMOD_OK )
	{
		printf( "ERROR: FMOD failed to find the inputed sound event. (%s) \n", FMOD_ErrorString( fmod_error_code ) );
		return false;
	}

	/* verify the sound event handle is valid */
	if( FMOD_Studio_EventDescription_IsValid( *sound_event ) == false )
	{
		printf( "ERROR: The sound event handle is invalid." );
		return false;
	}

return true;

} /* GetFMODSoundEvent */


/*******************************************************************
*
*   LoadBank()
*
*   DESCRIPTION:
*		create and intialize the fmod sound system.
*		returns true if suceeded, false if failed.
*
*******************************************************************/

static bool LoadBank( const char *filename, FMOD_STUDIO_SYSTEM *fmod_studio_system, FMOD_STUDIO_LOAD_BANK_FLAGS load_flags, FMOD_STUDIO_BANK **output_bank )
{
	char                    filepath[MAX_FILEPATH_LENGTH];

	for( uint32_t i = 0; i < cnt_of_array( RELATIVE_ROOT_DIRECTORY ); i++ )
	{
		sprintf_s( filepath, sizeof( filepath ), "%s%s", RELATIVE_ROOT_DIRECTORY[i], filename );
		
		fmod_error_code = FMOD_Studio_System_LoadBankFile( fmod_studio_system, filepath, load_flags, output_bank );
		if( fmod_error_code == FMOD_OK )
		{
			return true;
		}
	}

	printf( "ERROR: The FMOD bank failed to be loaded. (%s) \n", FMOD_ErrorString( fmod_error_code ) );
	return(false);

} /* LoadBank() */

// TODO jw make function for streaming sounds. aka music