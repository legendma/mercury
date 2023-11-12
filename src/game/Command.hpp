#pragma once

#include "ComponentClass.hpp"
#include "Universe.hpp"


namespace ECS
{
typedef enum _CommandProcessor
    {
    COMMAND_PROCESSOR_UNIVERSE,
    /* Count */
    COMMAND_PROCESSOR_COUNT
    } CommandProcessor;

typedef enum _CommandProcessorAction
    {
    COMMAND_PROCESSOR_ACTION_ADD,
    COMMAND_PROCESSOR_ACTION_REMOVE
    } CommandProcessorAction;

typedef void CommandProc( const ECS::PendingCommandComponent *command, ECS::Universe *universe );

bool Command_Init( ECS::Universe *universe );
void Command_Destroy( ECS::Universe *universe );
void Command_DoFrame( float frame_delta, ECS::Universe *universe );
void Command_AddCommandClass( const CommandProcessor processor, const PendingCommandClass command, CommandProcessorAction action, Universe *universe );
void Command_RegisterCommandProcessor( const CommandProcessor processor, CommandProc *processor_proc, Universe *universe );


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
component->u   = *command;

} /* Command_PostPending() */


/*******************************************************************
*
*   Command_MakeDestroyEntityCommand()
*
*   DESCRIPTION:
*       Helper function to fill out a 'destroy entity' command.
*
*******************************************************************/

static inline PendingCommandCommand * Command_MakeDestroyEntityCommand( const EntityId to_destroy, PendingCommandCommand *command )
{
*command = {};
command->destroy_entity.entity = to_destroy;

return( command );

} /* Command_MakeDestroyEntityCommand() */


} /* namespace ECS */