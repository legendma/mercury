#include <cassert>
#include <cstdlib>

#include "Command.hpp"
#include "Event.hpp"
#include "Universe.hpp"

using namespace ECS;

namespace Game
{

typedef enum _GameState
    {
    STATE_INTRO_SCENE,
    /* count */
    STATE_COUNT
    } GameState;

typedef struct _GameModeState
    {
    GameModeMainMode    main_mode;
    Universe           *universe;

    ///////////////// TESTING FOR HOT VARS - DON'T CHECK THIS IN <MPA> ///////////////////////
    bool chamber_opened;
    float malfoy_damage;
    s32 septum_sepra;

    ///////////////// TESTING FOR HOT VARS - DON'T CHECK THIS IN <MPA> ///////////////////////
    } GameModeState;


static GameModeState *  AsGameModeState( Universe *universe );
static void EnterIntro( GameModeState *system );
static void EnterState( const GameModeMainMode mode, GameModeState *system );
static void ExitState( const GameModeMainMode mode, GameModeState *system );
static CommandProcedure ProcessCommand;
static void             SwitchToNewMainMode( const GameModeMainMode new_mode, GameModeState *system );


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
component->ptr = (GameModeState*)malloc( sizeof(GameModeState) );

GameModeState *system = AsGameModeState( universe );
clr_struct( system );
system->main_mode = GAME_MODE_NONE;
system->universe  = universe;

Command_RegisterCommandProcessor( COMMAND_PROCESSOR_GAME_MODE, ProcessCommand, universe );
Command_AddCommandClass( COMMAND_PROCESSOR_GAME_MODE, PENDING_COMMAND_CHANGE_GAME_MODE, COMMAND_PROCESSOR_ACTION_ADD, universe );

PendingCommandCommand command;
Command_PostPending( PENDING_COMMAND_CHANGE_GAME_MODE, Command_MakeChangeGameMode( GAME_MODE_INTRO, &command ), universe );
Command_PostPending( PENDING_COMMAND_BIND_HOT_VAR, Command_MakeBindHotVarFloat( "/half_blood_prince/bathroom/malfoy_damage", &system->malfoy_damage, &command ), universe );

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
free( component->ptr );
component->ptr = NULL;

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
GameModeState *system = AsGameModeState( universe );

switch( system->main_mode )
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




///////////////// TESTING FOR HOT VARS - DON'T CHECK THIS IN <MPA> ///////////////////////
static bool one_time_unbind = false;
if( one_time_unbind )
    {
    PendingCommandCommand command;
    Command_PostPending( PENDING_COMMAND_BIND_HOT_VAR, Command_MakeUnbindHotVarFloat( "/half_blood_prince/bathroom/malfoy_damage", &system->malfoy_damage, &command ), system->universe );
    one_time_unbind = false;
    }
    
static bool one_time_bind = false;
if( one_time_bind )
    {
    PendingCommandCommand command;
    Command_PostPending( PENDING_COMMAND_BIND_HOT_VAR, Command_MakeBindHotVarFloat( "/half_blood_prince/bathroom/malfoy_damage", &system->malfoy_damage, &command ), system->universe );
    one_time_bind = false;
    }
    
static bool one_time_save = false;
if( one_time_save )
    {
    Command_PostPending( PENDING_COMMAND_SAVE_HOT_VARS, nullptr, system->universe );
    one_time_save = false;
    }
    
static bool one_time_load = false;
if( one_time_load )
    {
    Command_PostPending( PENDING_COMMAND_RELOAD_HOT_VARS, nullptr, system->universe );
    one_time_load = false;
    }

///////////////// TESTING FOR HOT VARS - DON'T CHECK THIS IN <MPA> ///////////////////////

} /* GameMode_DoFrame() */


/*******************************************************************
*
*   AsGameModeState()
*
*   DESCRIPTION:
*       Get the private system state.
*
*******************************************************************/

static GameModeState * AsGameModeState( Universe *universe )
{
SingletonGameModeComponent *component = (SingletonGameModeComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_GAME_MODE, universe );
return( (GameModeState*)component->ptr );

} /* AsGameModeState() */


/*******************************************************************
*
*   EnterIntro()
*
*   DESCRIPTION:
*       Enter the opening intro scene.
*
*******************************************************************/

static void EnterIntro( GameModeState *system )
{

} /* EnterIntro() /


/*******************************************************************
*
*   EnterState()
*
*   DESCRIPTION:
*       Handle opening of the given game main mode.
*
*******************************************************************/

static void EnterState( const GameModeMainMode mode, GameModeState *system )
{
switch( mode )
    {
    case GAME_MODE_INTRO:
        EnterIntro( system );
        break;

    default:
        debug_assert( false );
        break;
    }

} /* EnterState() /


/*******************************************************************
*
*   ExitState()
*
*   DESCRIPTION:
*       Handle closure of the given game main mode.
*
*******************************************************************/

static void ExitState( const GameModeMainMode mode, GameModeState *system )
{
switch( mode )
    {
    case GAME_MODE_INTRO:
        break;

    default:
        debug_assert( mode == GAME_MODE_NONE );
        break;
    }

} /* ExitState() /


/*******************************************************************
*
*   ProcessCommand()
*
*   DESCRIPTION:
*       Process a command.
*
*******************************************************************/

static void ProcessCommand( const PendingCommandComponent *command, Universe *universe )
{
GameModeState *system = AsGameModeState( universe );
switch( command->cls )
    {
    case PENDING_COMMAND_CHANGE_GAME_MODE:
        SwitchToNewMainMode( command->u.change_game_mode.new_mode, system );
        break;

    default:
        break;
    }

} /* ProcessCommand() */


/*******************************************************************
*
*   SwitchToNewMainMode()
*
*   DESCRIPTION:
*       Change main modes.
*
*******************************************************************/

static void SwitchToNewMainMode( const GameModeMainMode new_mode, GameModeState *system )
{
ExitState( system->main_mode, system );

GameModeMainMode old_mode = system->main_mode;
system->main_mode = new_mode;
EnterState( new_mode, system );

EventNotificationEvent evt;
Event_Enqueue( EVENT_NOTIFICATION_GAME_MAIN_MODE_CHANGED, Event_MakeChangeGameMainMode( old_mode, new_mode, &evt ), system->universe );

} /* SwitchToNewMainMode() */

} /* namespace Game */