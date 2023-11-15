#pragma once

#include "ComponentClass.hpp"
#include "Universe.hpp"


namespace ECS
{
typedef enum _EventListener
    {
    EVENT_LISTENER_RENDER,
    /* Count */
    EVENT_LISTENER_COUNT
    } EventListener;

typedef enum _EventListenAction
    {
    EVENT_LISTEN_ACTION_START_LISTENING,
    EVENT_LISTEN_ACTION_STOP_LISTENING
    } EventListenAction;

typedef void EventProcedure( const ECS::EventNotificationComponent *evt, ECS::Universe *universe );

bool Event_Init( ECS::Universe *universe );
void Event_Destroy( ECS::Universe *universe );
void Event_DoFrame( float frame_delta, ECS::Universe *universe );
void Event_ListenToEvent( const EventListener listener, const EventNotificationClass evt, EventListenAction action, Universe *universe );
void Event_RegisterEventListener( const EventListener listener, EventProcedure *handler_proc, Universe *universe );


/*******************************************************************
*
*   Event_Enqueue()
*
*   DESCRIPTION:
*       Helper function to enqueue an event to be processed at the
*       end of the frame.
*
*******************************************************************/

static inline void Event_Enqueue( const EventNotificationClass cls, const EventNotificationEvent *evt, Universe *universe )
{
EntityId entity = Universe_CreateNewEntity( universe );
EventNotificationComponent *component = (EventNotificationComponent*)Universe_AttachComponentToEntity( entity, COMPONENT_EVENT_NOTIFICATION, universe );

component->cls = cls;
component->u   = *evt;

} /* Event_Enqueue() */


/*******************************************************************
*
*   Event_MakeChangeGameMainMode()
*
*   DESCRIPTION:
*       Helper function to create a new event for the main game mode
*       changing.
*
*******************************************************************/

static inline EventNotificationEvent * Event_MakeChangeGameMainMode( const GameModeMainMode from_old, const GameModeMainMode to_new, EventNotificationEvent *evt )
{
*evt = {};
evt->game_mode_changed.from_old = from_old;
evt->game_mode_changed.to_new   = to_new;

return( evt );

} /* Event_MakeChangeGameMainMode() */




} /* namespace ECS */