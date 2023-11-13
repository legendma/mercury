#include <cassert>

#include "Command.hpp"
#include "Event.hpp" // TODO <MPA>
#include "Universe.hpp"

using namespace ECS;

static CommandProcedure ProcessCommand;


/*******************************************************************
*
*   GameMode_Init()
*
*   DESCRIPTION:
*       Initialize the game mode system.
*       Returns TRUE if the system successfully initialized.
*
*******************************************************************/

bool GameMode_Init( Universe *universe )
{
SingletonGameModeComponent *component = (SingletonGameModeComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_GAME_MODE, universe );

component->main_mode = GAME_MODE_NONE;
Command_RegisterCommandProcessor( COMMAND_PROCESSOR_GAME_MODE, ProcessCommand, universe );
Command_AddCommandClass( COMMAND_PROCESSOR_GAME_MODE, PENDING_COMMAND_CHANGE_GAME_MODE, COMMAND_PROCESSOR_ACTION_ADD, universe );

PendingCommandCommand command;
Command_PostPending( PENDING_COMMAND_CHANGE_GAME_MODE, Command_MakeChangeGameMode( GAME_MODE_INTRO, &command ), universe);

return( true );

} /* GameMode_Init() */


/*******************************************************************
*
*   GameMode_Destroy()
*
*   DESCRIPTION:
*       Destroy the Game Mode system and free its resources.
*
*******************************************************************/

void GameMode_Destroy( Universe *universe )
{
SingletonGameModeComponent *component = (SingletonGameModeComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_GAME_MODE, universe );


} /* GameMode_Destroy() */


/*******************************************************************
*
*   GameMode_DoFrame()
*
*   DESCRIPTION:
*       Advance the system one frame.
*
*******************************************************************/

void GameMode_DoFrame( float frame_delta, Universe *universe )
{
SingletonGameModeComponent *component = (SingletonGameModeComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_GAME_MODE, universe );

// TODO <MPA> - REMOVE TEST CODE
EventNotificationEvent evt = {};
Event_Enqueue( EVENT_NOTIFICATION_DUMMY_REMOVE_ME, &evt, universe );

// END TEST CODE

switch( component->main_mode )
    {
    case GAME_MODE_NONE:
        break;

    case GAME_MODE_INTRO:
        break;

    case GAME_MODE_MAIN_MENU:
        break;

    case GAME_MODE_CHARACTER_CREATE:
        break;

    case GAME_MODE_IN_GAME:
        break;

    default:
        assert( false );
        break;
    }

} /* GameMode_DoFrame() */


/*******************************************************************
*
*   ProcessCommand()
*
*   DESCRIPTION:
*       Process a command.
*
*******************************************************************/

void ProcessCommand( const ECS::PendingCommandComponent *command, ECS::Universe *universe )
{
switch( command->cls )
    {
    case PENDING_COMMAND_CHANGE_GAME_MODE:
        break;


    }

} /* ProcessCommand() */

