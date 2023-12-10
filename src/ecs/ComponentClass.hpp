#pragma once

#include <cstdint>

#include "AssetFile.hpp"
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
    Float2                axis_state [CONTROLLER_AXIS_COUNT ];
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
*   COMPONENT_SINGLETON_SOUND_SYSTEM - SingletonSoundSystemComponent
*
*******************************************************************/

IMPLEMENT_SINGLETON_VOID( SingletonSoundSystemComponent );

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

IMPLEMENT_SINGLETON_VOID( SingletonGameModeComponent );

/*******************************************************************
*
*   COMPONENT_EVENT_NOTIFICATION - EventNotificationComponent
*
*******************************************************************/

typedef enum _EventNotificationClass
    {
    EVENT_NOTIFICATION_GAME_MAIN_MODE_CHANGED,
    /* count */
    EVENT_NOTIFICATION_CLASS_COUNT
    } EventNotificationClass;

typedef union _EventNotificationEvent
    {
    struct
        {
        GameModeMainMode
                        from_old;
        GameModeMainMode
                        to_new;
        } game_mode_changed;        /* EVENT_NOTIFICATION_GAME_MAIN_MODE_CHANGED */
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
*   COMPONENT_MODEL - ModelComponent
*
*******************************************************************/

typedef struct _ModelComponent
    {
    uint32_t            scene_name_hash;
    AssetFileNameString asset_name;
    } ModelComponent;

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
    /* PENDING_COMMAND_CHANGE_GAME_MODE */
    struct
        {
        GameModeMainMode
                        new_mode;
        } change_game_mode;

    /* PENDING_COMMAND_DESTROY_ENTITY */
    struct
        {
        EntityId        entity;
        } destroy_entity;

    /* PENDING_COMMAND_LOAD_MODEL */
    struct
        {
        EntityId        entity;
        AssetFileNameString
                        asset_name;
        } load_model;
    } PendingCommandCommand;

typedef struct _PendingCommandComponent
    {
    PendingCommandClass cls;
    PendingCommandCommand
                        u;
    } PendingCommandComponent;

/*******************************************************************
*
*   COMPONENT_SCENE - SceneComponent
*
*******************************************************************/

#define SCENE_COMPONENT_NAME_MAX_LEN \
                                    ( 20 )

typedef struct _SceneComponent
    {
    Float2              viewport_top_left;
    Float2              viewport_extent;
    uint64_t            draw_order;
    char                scene_name[ SCENE_COMPONENT_NAME_MAX_LEN ];
    uint32_t            scene_name_hash;
    void               *render_state;
    } SceneComponent;

/*******************************************************************
*
*   COMPONENT_SOUNDS - SoundsComponent
*
*******************************************************************/

typedef struct _SoundsComponent
    {
    void               *event_description;
    void               *event_instance;
    } SoundsComponent;

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
*   COMPONENT_TRANSFORM - TransformComponent
*
*******************************************************************/

typedef struct _TransformComponent
    {
    Float3              position;
    Quaternion          rotation;
    Float3              scale;
    } TransformComponent;

/*******************************************************************
*
*   Component Classes
*
*******************************************************************/

typedef enum
    {
    COMPONENT_EVENT_NOTIFICATION,
    COMPONENT_MODEL,
    COMPONENT_PENDING_COMMAND,
    COMPONENT_SCENE,
    COMPONENT_SINGLETON_COMMAND,
    COMPONENT_SINGLETON_CONTROLLER_INPUT,
    COMPONENT_SINGLETON_GAME_MODE,
    COMPONENT_SINGLETON_EVENT,
    COMPONENT_SINGLETON_PLAYER_INPUT,
    COMPONENT_SINGLETON_RENDER,
    COMPONENT_SINGLETON_SOUND_SYSTEM,
    COMPONENT_SOUNDS,
    COMPONENT_TRANSFORM,
    /* count */
    COMPONENT_CNT
    } ComponentClass;
    
typedef struct _ComponentClassSizes
    {
    ComponentClass      cls;
    size_t              size;
    } ComponentClassSizes;

static const ComponentClassSizes COMPONENT_CLASS_SIZES[] =
    {
    { COMPONENT_EVENT_NOTIFICATION,         sizeof( EventNotificationComponent )        },
    { COMPONENT_MODEL,                      sizeof( ModelComponent )                    },
    { COMPONENT_PENDING_COMMAND,            sizeof( PendingCommandComponent )           },
    { COMPONENT_SCENE,                      sizeof( SceneComponent )                    },
    { COMPONENT_SINGLETON_COMMAND,          sizeof( SingletonCommandComponent )         },
    { COMPONENT_SINGLETON_CONTROLLER_INPUT, sizeof( SingletonControllerInputComponent ) },
    { COMPONENT_SINGLETON_EVENT,            sizeof( SingletonEventComponent )           },
    { COMPONENT_SINGLETON_GAME_MODE,        sizeof( SingletonGameModeComponent )        },
    { COMPONENT_SINGLETON_PLAYER_INPUT,     sizeof( SingletonPlayerInputComponent )     },
    { COMPONENT_SINGLETON_RENDER,           sizeof( SingletonRenderComponent )          },
    { COMPONENT_SINGLETON_SOUND_SYSTEM,     sizeof( SingletonSoundSystemComponent )     },
    { COMPONENT_SOUNDS,                     sizeof( SoundsComponent )                   },
    { COMPONENT_TRANSFORM,                  sizeof( TransformComponent )                }
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