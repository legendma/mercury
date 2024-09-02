#pragma once

#include "ComponentClass.hpp"
#include "Universe.hpp"


namespace ECS
{

typedef enum _CommandProcessor
    {
    COMMAND_PROCESSOR_GAME_MODE,
    COMMAND_PROCESSOR_HOT_VARS,
    COMMAND_PROCESSOR_UNIVERSE,
    /* Count */
    COMMAND_PROCESSOR_COUNT
    } CommandProcessor;

typedef enum _CommandProcessorAction
    {
    COMMAND_PROCESSOR_ACTION_ADD,
    COMMAND_PROCESSOR_ACTION_REMOVE
    } CommandProcessorAction;

typedef void CommandProcedure( const ECS::PendingCommandComponent *command, ECS::Universe *universe );

bool Command_Init( ECS::Universe *universe );
void Command_Destroy( ECS::Universe *universe );
void Command_DoFrame( float frame_delta, ECS::Universe *universe );
void Command_AddCommandClass( const CommandProcessor processor, const PendingCommandClass command, CommandProcessorAction action, Universe *universe );
void Command_RegisterCommandProcessor( const CommandProcessor processor, CommandProcedure *processor_proc, Universe *universe );


/*******************************************************************
*
*   Command_PostPending()
*
*   DESCRIPTION:
*       Helper function to post a command to be handled at the end
*       of the frame.
*
*******************************************************************/

static inline void Command_PostPending( const PendingCommandClass cls, const PendingCommandCommand *command, Universe *universe )
{
EntityId entity = Universe_CreateNewEntity( universe );
PendingCommandComponent *component = (PendingCommandComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_PENDING_COMMAND, universe );

component->cls = cls;
if( command )
    {
    component->u = *command;
    }

} /* Command_PostPending() */


/*******************************************************************
*
*   Command_MakeBindHotVarBool()
*
*   DESCRIPTION:
*       Helper function to fill out a 'bind bool hot var' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeBindHotVarBool( const char *name, bool *binding, PendingCommandCommand *command )
{
*command = {};
command->hot_vars_bind.is_bind = true;

command->hot_vars_bind.request.name   = name;
command->hot_vars_bind.request.type   = HOT_VARS_BOOL;
command->hot_vars_bind.request.bind.b = binding;

return( command );

} /* Command_MakeBindHotVarBool() */


/*******************************************************************
*
*   Command_MakeBindHotVarFloat()
*
*   DESCRIPTION:
*       Helper function to fill out a 'bind float hot var' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeBindHotVarFloat( const char *name, f32 *binding, PendingCommandCommand *command )
{
*command = {};
command->hot_vars_bind.is_bind = true;

command->hot_vars_bind.request.name   = name;
command->hot_vars_bind.request.type   = HOT_VARS_FLOAT;
command->hot_vars_bind.request.bind.f = binding;

return( command );

} /* Command_MakeBindHotVarFloat() */


/*******************************************************************
*
*   Command_MakeBindHotVarSint()
*
*   DESCRIPTION:
*       Helper function to fill out a 'bind sint hot var' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeBindHotVarSint( const char *name, s32 *binding, PendingCommandCommand *command )
{
*command = {};
command->hot_vars_bind.is_bind = true;

command->hot_vars_bind.request.name   = name;
command->hot_vars_bind.request.type   = HOT_VARS_SINT;
command->hot_vars_bind.request.bind.s = binding;

return( command );

} /* Command_MakeBindHotVarSint() */


/*******************************************************************
*
*   Command_MakeChangeGameMode()
*
*   DESCRIPTION:
*       Helper function to fill out a 'change game mode' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeChangeGameMode( const GameModeMainMode change_to, PendingCommandCommand *command )
{
*command = {};
command->change_game_mode.new_mode = change_to;

return( command );

} /* Command_MakeChangeGameMode() */


/*******************************************************************
*
*   Command_MakeDestroyEntity()
*
*   DESCRIPTION:
*       Helper function to fill out a 'destroy entity' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeDestroyEntity( const EntityId to_destroy, PendingCommandCommand *command )
{
*command = {};
command->destroy_entity.entity = to_destroy;

return( command );

} /* Command_MakeDestroyEntity() */


/*******************************************************************
*
*   Command_MakeUnbindHotVarBool()
*
*   DESCRIPTION:
*       Helper function to fill out a 'unbind bool hot var' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeUnbindHotVarBool( const char *name, bool *binding, PendingCommandCommand *command )
{
*command = {};
command->hot_vars_bind.is_bind = false;

command->hot_vars_bind.request.name   = name;
command->hot_vars_bind.request.type   = HOT_VARS_BOOL;
command->hot_vars_bind.request.bind.b = binding;

return( command );

} /* Command_MakeUnbindHotVarBool() */


/*******************************************************************
*
*   Command_MakeUnbindHotVarFloat()
*
*   DESCRIPTION:
*       Helper function to fill out a 'unbind float hot var'
*       command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeUnbindHotVarFloat( const char *name, f32 *binding, PendingCommandCommand *command )
{
*command = {};
command->hot_vars_bind.is_bind = false;

command->hot_vars_bind.request.name   = name;
command->hot_vars_bind.request.type   = HOT_VARS_FLOAT;
command->hot_vars_bind.request.bind.f = binding;

return( command );

} /* Command_MakeUnbindHotVarFloat() */


/*******************************************************************
*
*   Command_MakeUnbindHotVarSint()
*
*   DESCRIPTION:
*       Helper function to fill out a 'unbind sint hot var' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeUnbindHotVarSint( const char *name, s32 *binding, PendingCommandCommand *command )
{
*command = {};
command->hot_vars_bind.is_bind = false;

command->hot_vars_bind.request.name   = name;
command->hot_vars_bind.request.type   = HOT_VARS_SINT;
command->hot_vars_bind.request.bind.s = binding;

return( command );

} /* Command_MakeUnbindHotVarSint() */


} /* namespace ECS */