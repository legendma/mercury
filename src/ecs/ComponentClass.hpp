#pragma once

#include <cstdint>

#include "AssetFile.hpp"
#include "Entity.hpp"
#include "Global.hpp"
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
    Float2              axis_state[ CONTROLLER_AXIS_COUNT ];
    float               trigger_state[ CONTROLLER_TRIGGERS_COUNT ];
    ControllerButtonQuery
                       *is_pressed[ CONTROLLER_BUTTON_COUNT ];
    } SingletonControllerInputComponent;

/*******************************************************************
*
*   COMPONENT_SINGLETON_KEYBOARD_INPUT - SingletonKeyboardInputComponent
*
*******************************************************************/

typedef enum _KeyboardKeyScanCode
    {
    KEYBOARD_KEY_NULL = 0,
    KEYBOARD_KEY_ESC = 1,
    KEYBOARD_KEY_1 = 2,
    KEYBOARD_KEY_2 = 3,
    KEYBOARD_KEY_3 = 4,
    KEYBOARD_KEY_4 = 5,
    KEYBOARD_KEY_5 = 6,
    KEYBOARD_KEY_6 = 7,
    KEYBOARD_KEY_7 = 8,
    KEYBOARD_KEY_8 = 9,
    KEYBOARD_KEY_9 = 10,
    KEYBOARD_KEY_0 = 11,
    KEYBOARD_KEY_HYPHEN = 12,
    KEYBOARD_KEY_EQUALS = 13,
    KEYBOARD_KEY_BACKSPACE = 14,
    KEYBOARD_KEY_TAB = 15,
    KEYBOARD_KEY_Q = 16,
    KEYBOARD_KEY_W = 17,
    KEYBOARD_KEY_E = 18,
    KEYBOARD_KEY_R = 19,
    KEYBOARD_KEY_T = 20,
    KEYBOARD_KEY_Y = 21,
    KEYBOARD_KEY_U = 22,
    KEYBOARD_KEY_I = 23,
    KEYBOARD_KEY_O = 24,
    KEYBOARD_KEY_P = 25,
    KEYBOARD_KEY_LEFT_BRACKET = 26,
    KEYBOARD_KEY_RIGHT_BRACKET = 27,
    KEYBOARD_KEY_ENTER = 28,
    KEYBOARD_KEY_CTRL = 29,
    KEYBOARD_KEY_A = 30,
    KEYBOARD_KEY_S = 31,
    KEYBOARD_KEY_D = 32,
    KEYBOARD_KEY_F = 33,
    KEYBOARD_KEY_G = 34,
    KEYBOARD_KEY_H = 35,
    KEYBOARD_KEY_J = 36,
    KEYBOARD_KEY_K = 37,
    KEYBOARD_KEY_L = 38,
    KEYBOARD_KEY_SEMI_COLON = 39,
    KEYBOARD_KEY_APOSTROPHE = 40,
    KEYBOARD_KEY_BACK_QUOTE = 41,
    KEYBOARD_KEY_LEFT_SHIFT = 42,
    KEYBOARD_KEY_BACKSLASH = 43,
    KEYBOARD_KEY_Z = 44,
    KEYBOARD_KEY_X = 45,
    KEYBOARD_KEY_C = 46,
    KEYBOARD_KEY_V = 47,
    KEYBOARD_KEY_B = 48,
    KEYBOARD_KEY_N = 49,
    KEYBOARD_KEY_M = 50,
    KEYBOARD_KEY_COMMA = 51,
    KEYBOARD_KEY_PERIOD = 52,
    KEYBOARD_KEY_FORWARD_SLASH = 53,
    KEYBOARD_KEY_RIGHT_SHIFT = 54,
    KEYBOARD_KEY_PRINT_SCREEN = 55,
    KEYBOARD_KEY_ALT = 56,
    KEYBOARD_KEY_SPACE = 57,
    KEYBOARD_KEY_CAPSLOCK = 58,
    KEYBOARD_KEY_F1 = 59,
    KEYBOARD_KEY_F2 = 60,
    KEYBOARD_KEY_F3 = 61,
    KEYBOARD_KEY_F4 = 62,
    KEYBOARD_KEY_F5 = 63,
    KEYBOARD_KEY_F6 = 64,
    KEYBOARD_KEY_F7 = 65,
    KEYBOARD_KEY_F8 = 66,
    KEYBOARD_KEY_F9 = 67,
    KEYBOARD_KEY_F10 = 68,
    KEYBOARD_KEY_NUMLOCK = 69,
    KEYBOARD_KEY_SCROLL_LOCK = 70,
    KEYBOARD_KEY_HOME = 71,
    KEYBOARD_KEY_UP = 72,
    KEYBOARD_KEY_PAGE_UP = 73,
    KEYBOARD_KEY_MINUS = 74,
    KEYBOARD_KEY_LEFT = 75,
    KEYBOARD_KEY_CENTER = 76,
    KEYBOARD_KEY_RIGHT = 77,
    KEYBOARD_KEY_PLUS = 78,
    KEYBOARD_KEY_END = 79,
    KEYBOARD_KEY_DOWN = 80,
    KEYBOARD_KEY_PAGE_DOWN = 81,
    KEYBOARD_KEY_INSERT = 82,
    KEYBOARD_KEY_DELETE = 83,
    /* count */
    KEYBOARD_KEY_COUNT 
    } KeyboardKeyScanCode;

typedef uint8_t SingletonKeyboardKeyBAType;
#define SINGLETON_KEYBOARD_KEY_BA_COUNT \
    MATH_BITARRAY_COUNT( SingletonKeyboardKeyBAType, KEYBOARD_KEY_COUNT )

typedef struct _SingletonKeyboardKeyBitArray
{
    SingletonKeyboardKeyBAType
        ba[SINGLETON_KEYBOARD_KEY_BA_COUNT];
} SingletonKeyboardKeyBitArray;


typedef struct _SingletonKeyboardInputComponent
    {
    SingletonKeyboardKeyBitArray keyboard_key_pressed;
    SingletonKeyboardKeyBitArray keyboard_key_changed;
    uint8_t                      number_of_keys_pressed;

    } SingletonKeyboardInputComponent;

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
*   COMPONENT_SINGLETON_HOT_VARS - SingletonHotVarsComponent
*
*******************************************************************/

typedef enum
    {
    HOT_VARS_BOOL,
    HOT_VARS_SINT,
    HOT_VARS_FLOAT,
    /* internal only */
    HOT_VARS_DIRECTORY
    } HotVarsDataType;

typedef struct
    {
    const char         *name;
    HotVarsDataType     type;
    union
        {
        bool           *b;
        f32            *f;
        s32            *s;
        } bind;
    } HotVarBinding;

IMPLEMENT_SINGLETON_VOID( SingletonHotVarsComponent );

/*******************************************************************
*
*   COMPONENT_HOT_VAR_BINDING - HotVarBindingComponent
*
*******************************************************************/

typedef struct
    {
    HotVarBinding       binding;
    } HotVarBindingComponent;

/*******************************************************************
*
*   COMPONENT_HOT_VAR_DEFINITION - HotVarDefinitionComponent
*
*******************************************************************/

typedef struct
    {
    const char         *name;
    HotVarsDataType     type;
    EntityId            directory;
    union
        {
        bool            b;
        f32             f;
        s32             s;
        } value;
    } HotVarDefinitionComponent;

/*******************************************************************
*
*   COMPONENT_EVENT_NOTIFICATION - EventNotificationComponent
*
*******************************************************************/

typedef enum _EventNotificationClass
    {
    EVENT_NOTIFICATION_GAME_MAIN_MODE_CHANGED,
    EVENT_NOTIFICATION_HOT_VARS_RELOADED,
    EVENT_NOTIFICATION_HOT_VARS_SAVED,
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

    struct
        {
        bool          is_going_up;     // otherwise is going down
        } keyboard_key_changed;        /* EVENT_NOTIFICATION_KEYBOARD_KEY_CHANGED_* */
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
    PENDING_COMMAND_BIND_HOT_VAR,
    PENDING_COMMAND_CHANGE_GAME_MODE,
    PENDING_COMMAND_DESTROY_ENTITY,
    PENDING_COMMAND_RELOAD_HOT_VARS,
    PENDING_COMMAND_SAVE_HOT_VARS,
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

    /* PENDING_COMMAND_BIND_HOT_VAR */
    struct
        {
        bool            is_bind;
        HotVarBinding   request;
        } hot_vars_bind;
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
    COMPONENT_HOT_VAR_BINDING,
    COMPONENT_HOT_VAR_DEFINITION,
    COMPONENT_MODEL,
    COMPONENT_PENDING_COMMAND,
    COMPONENT_SCENE,
    COMPONENT_SINGLETON_COMMAND,
    COMPONENT_SINGLETON_CONTROLLER_INPUT,
    COMPONENT_SINGLETON_KEYBOARD_INPUT,
    COMPONENT_SINGLETON_GAME_MODE,
    COMPONENT_SINGLETON_EVENT,
    COMPONENT_SINGLETON_HOT_VARS,
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
    { COMPONENT_HOT_VAR_BINDING,            sizeof( HotVarBindingComponent )            },
    { COMPONENT_HOT_VAR_DEFINITION,         sizeof( HotVarDefinitionComponent )         },
    { COMPONENT_MODEL,                      sizeof( ModelComponent )                    },
    { COMPONENT_PENDING_COMMAND,            sizeof( PendingCommandComponent )           },
    { COMPONENT_SCENE,                      sizeof( SceneComponent )                    },
    { COMPONENT_SINGLETON_COMMAND,          sizeof( SingletonCommandComponent )         },
    { COMPONENT_SINGLETON_CONTROLLER_INPUT, sizeof( SingletonControllerInputComponent ) },
    { COMPONENT_SINGLETON_KEYBOARD_INPUT,   sizeof( SingletonKeyboardInputComponent )   },
    { COMPONENT_SINGLETON_EVENT,            sizeof( SingletonEventComponent )           },
    { COMPONENT_SINGLETON_HOT_VARS,         sizeof( SingletonHotVarsComponent )         },
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