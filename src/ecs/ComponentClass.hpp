#pragma once

#include <cstdint>

#include "Entity.hpp"
#include "Math.hpp"
#include "Utilities.hpp"

namespace ECS
{
/*******************************************************************
*
*   IMPLEMENT_SINGLETON_VOID()
*
*   DESCRIPTION:
*       Implement the standard singleton component containing only
*       a void *.
*
*******************************************************************/

#define IMPLEMENT_SINGLETON_VOID( _type ) \
    typedef struct _##_type               \
        {                                 \
        void               *ptr;          \
        } _type


/*******************************************************************
*
*   COMPONENT_SINGLETON_CONTROLLER_INPUT - SingletonControllerInputComponent
*
*******************************************************************/

typedef enum _ControllerButtons 
    {
    CONTROLLER_BUTTON_DPAD_CENTER,
    CONTROLLER_BUTTON_DPAD_UP,
    CONTROLLER_BUTTON_DPAD_UPRIGHT,
    CONTROLLER_BUTTON_DPAD_RIGHT,
    CONTROLLER_BUTTON_DPAD_DOWNRIGHT,
    CONTROLLER_BUTTON_DPAD_DOWN,
    CONTROLLER_BUTTON_DPAD_DOWNLEFT,
    CONTROLLER_BUTTON_DPAD_LEFT,
    CONTROLLER_BUTTON_DPAD_UPLEFT,
    CONTROLLER_BUTTON_LEFT_STICK_CLICK,
    CONTROLLER_BUTTON_RIGHT_STICK_CLICK,
    CONTROLLER_BUTTON_L1,
    CONTROLLER_BUTTON_R1,
    CONTROLLER_BUTTON_L2,
    CONTROLLER_BUTTON_R2,
    CONTROLLER_BUTTON_MENU,
    CONTROLLER_BUTTON_OPTIONS,
    CONTROLLER_BUTTON_CENTER_PAD,
    CONTROLLER_BUTTON_POWER,
    CONTROLLER_BUTTON_MUTE,
    CONTROLLER_BUTTON_BOT_BUTTON,
    CONTROLLER_BUTTON_TOP_BUTTON,
    CONTROLLER_BUTTON_LEFT_BUTTON,
    CONTROLLER_BUTTON_RIGHT_BUTTON,
    /* Count */
    CONTROLLER_BUTTON_COUNT
    } ControllerButtons;

typedef enum _ControllerAxis
    {
    CONTROLLER_AXIS_LEFT_STICK,
    CONTROLLER_AXIS_RIGHT_STICK,
    /* Count */
    CONTROLLER_AXIS_COUNT
    } ControllerAxis;

typedef enum _ControllerTriggers
    {
    CONTROLLER_TRIGGERS_LEFT,
    CONTROLLER_TRIGGERS_RIGHT,
    /* count */
    CONTROLLER_TRIGGERS_COUNT
    } ControllerTriggers;

typedef uint8_t SingletonControllerInputButtonStateBAType;
#define SINGLETON_CONTROLLER_INPUT_BUTTON_BA_COUNT \
    MATH_BITARRAY_COUNT( SingletonControllerInputButtonStateBAType, CONTROLLER_BUTTON_COUNT )

typedef struct _SingletonControllerInputButtonStateBitArray
    {
    SingletonControllerInputButtonStateBAType
                        ba[ SINGLETON_CONTROLLER_INPUT_BUTTON_BA_COUNT ];
    } SingletonControllerInputButtonStateBitArray;

struct _SingletonControllerInputComponent;
typedef bool ControllerButtonQuery( const _SingletonControllerInputComponent *component );

typedef struct _SingletonControllerInputComponent
    {
    SingletonControllerInputButtonStateBitArray
                        button_state;
    Vec2                axis_state [CONTROLLER_AXIS_COUNT ];
    float               trigger_state [CONTROLLER_TRIGGERS_COUNT ];
    ControllerButtonQuery
                       *is_pressed[ CONTROLLER_BUTTON_COUNT ];
    } SingletonControllerInputComponent;

/*******************************************************************
*
*   COMPONENT_SINGLETON_PLAYER_INPUT - SingletonPlayerInputComponent
*
*******************************************************************/

IMPLEMENT_SINGLETON_VOID( SingletonPlayerInputComponent );

/*******************************************************************
*
*   COMPONENT_SINGLETON_RENDER - SingletonRenderComponent
*
*******************************************************************/

IMPLEMENT_SINGLETON_VOID( SingletonRenderComponent );

/*******************************************************************
*
*   COMPONENT_SINGLETON_GAME_MODE - SingletonGameModeComponent
*
*******************************************************************/

typedef enum _GameModeMainMode
    {
    GAME_MODE_NONE,
    GAME_MODE_INTRO,
    GAME_MODE_MAIN_MENU,
    GAME_MODE_CHARACTER_CREATE,
    GAME_MODE_IN_GAME
    } GameModeMainMode;

typedef struct _SingletonGameModeComponent
    {
    GameModeMainMode    main_mode;
    } SingletonGameModeComponent;

/*******************************************************************
*
*   COMPONENT_EVENT_NOTIFICATION - EventNotificationComponent
*
*******************************************************************/

typedef enum _EventNotificationClass
    {
    EVENT_NOTIFICATION_DUMMY_REMOVE_ME,
    /* count */
    EVENT_NOTIFICATION_CLASS_COUNT
    } EventNotificationClass;

typedef union _EventNotificationEvent
    {
    int i;
    } EventNotificationEvent;

typedef struct _EventNotificationComponent
    {
    EventNotificationClass
                        cls;
    EventNotificationEvent
                        u;
    } EventNotificationComponent;

/*******************************************************************
*
*   COMPONENT_PENDING_COMMAND - PendingCommandComponent
*
*******************************************************************/

typedef enum _PendingCommandClass
    {
    PENDING_COMMAND_CHANGE_GAME_MODE,
    PENDING_COMMAND_DESTROY_ENTITY,
    /* count */
    PENDING_COMMAND_CLASS_COUNT
    } PendingCommandClass;

typedef union _PendingCommandCommand
    {
    struct
        {
        EntityId        entity;
        } destroy_entity;           /* PENDING_COMMAND_DESTROY_ENTITY */
    struct
        {
        GameModeMainMode
                        new_mode;
        } change_game_mode;         /* PENDING_COMMAND_CHANGE_GAME_MODE */
    } PendingCommandCommand;

typedef struct _PendingCommandComponent
    {
    PendingCommandClass cls;
    PendingCommandCommand
                        u;
    } PendingCommandComponent;

/*******************************************************************
*
*   COMPONENT_SINGLETON_COMMAND - SingletonCommandComponent
*
*******************************************************************/

IMPLEMENT_SINGLETON_VOID( SingletonCommandComponent );

/*******************************************************************
*
*   COMPONENT_SINGLETON_EVENT - SingletonEventComponent
*
*******************************************************************/

IMPLEMENT_SINGLETON_VOID( SingletonEventComponent );

/*******************************************************************
*
*   Component Classes
*
*******************************************************************/

typedef enum
    {
    COMPONENT_EVENT_NOTIFICATION,
    COMPONENT_PENDING_COMMAND,
    COMPONENT_SINGLETON_COMMAND,
    COMPONENT_SINGLETON_CONTROLLER_INPUT,
    COMPONENT_SINGLETON_GAME_MODE,
    COMPONENT_SINGLETON_EVENT,
    COMPONENT_SINGLETON_PLAYER_INPUT,
    COMPONENT_SINGLETON_RENDER,
    /* count */
    COMPONENT_CNT
    } ComponentClass;
    
typedef struct _ComponentClassSizes
    {
    ComponentClass      cls;
    size_t              size;
    } ComponentClassSizes;

static const ComponentClassSizes COMPONENT_CLASS_SIZES[] = /* TODO <MPA> - If in the future this table needs to be referenced in update() time, need to store it in the universe as size_t array[ COMPONENT_CNT ] for quick lookup */
    {
    { COMPONENT_EVENT_NOTIFICATION,         sizeof( EventNotificationComponent )        },
    { COMPONENT_PENDING_COMMAND,            sizeof( PendingCommandComponent )           },
    { COMPONENT_SINGLETON_COMMAND,          sizeof( SingletonCommandComponent )         },
    { COMPONENT_SINGLETON_CONTROLLER_INPUT, sizeof( SingletonControllerInputComponent ) },
    { COMPONENT_SINGLETON_EVENT,            sizeof( SingletonEventComponent )           },
    { COMPONENT_SINGLETON_GAME_MODE,        sizeof( SingletonGameModeComponent )        },
    { COMPONENT_SINGLETON_PLAYER_INPUT,     sizeof( SingletonPlayerInputComponent )     },
    { COMPONENT_SINGLETON_RENDER,           sizeof( SingletonRenderComponent )          }
    };
compiler_assert( cnt_of_array( COMPONENT_CLASS_SIZES ) == COMPONENT_CNT, component_class_hpp );


/*******************************************************************
*
*   GetComponentClassSize()
*
*   DESCRIPTION:
*       Handy helper method to search for a component class's
*       component size.
*
*******************************************************************/

static inline size_t GetComponentClassSize( const ComponentClass component )
{
size_t ret = 0;
for( uint32_t i = 0; i < cnt_of_array( COMPONENT_CLASS_SIZES ); i++ )
    {
    if( COMPONENT_CLASS_SIZES[ i ].cls == component )
        {
        ret = COMPONENT_CLASS_SIZES[ i ].size;
        break;
        }
    }

return( ret );

}   /* GetComponentClassSize() */


} /* namespace ECS */