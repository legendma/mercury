#include <GameInput.h>
#include <cstdio>

#include "universe.hpp"

using namespace ECS;


typedef struct _DirectXPlayerInput
    {
    IGameInput* game_input_library = nullptr;
    IGameInputDevice* the_gamepad = nullptr;
    } DirectXPlayerInput;

//DirectXPlayerInput global_remove_me;

typedef enum _DualSenseReadAxis
    {
    DUALSENSEREADAXIS_LEFT_STICK_X,
    DUALSENSEREADAXIS_LEFT_STICK_Y,
    DUALSENSEREADAXIS_RIGHT_STICK_X,
    DUALSENSEREADAXIS_LEFT_TRIGGER,
    DUALSENSEREADAXIS_RIGHT_TRIGGER,
    DUALSENSEREADAXIS_RIGHT_STICK_Y,
    /* count */
    DUALSENSEREADAXIS_COUNT,
    }DualSenseReadAxis;


typedef enum _DualSenseButtons
{
    DUALSENSEBUTTONS_LEFT_BUTTON,
    DUALSENSEBUTTONS_BOTTOM_BUTTON,
    DUALSENSEBUTTONS_RIGHT_BUTTON,
    DUALSENSEBUTTONS_TOP_BUTTON,
    DUALSENSEBUTTONS_L1_TRIGGER,
    DUALSENSEBUTTONS_R1_TRIGGER,
    DUALSENSEBUTTONS_L2_TRIGGER,
    DUALSENSEBUTTONS_R2_TRIGGER,
    DUALSENSEBUTTONS_OPTION,
    DUALSENSEBUTTONS_START,
    DUALSENSEBUTTONS_LEFT_STICK_CLICK,
    DUALSENSEBUTTONS_RIGHT_STICK_CLICK,
    DUALSENSEBUTTONS_POWER_BUTTON,
    DUALSENSEBUTTONS_CENTER_BUTTON,
    DUALSENSEBUTTONS_MUTE_BUTTON,
    /* count */
    DUALSENSEBUTTONS__COUNT,
} DualSenseButtons;

typedef enum _DualSenseDPad
    {
    DUALSENSEDPAD_UP,
    DUALSENSEDPAD_UPRIGHT,
    DUALSENSEDPAD_RIGHT,
    DUALSENSEDPAD_DOWNRIGHT,
    DUALSENSEDPAD_DOWN,
    DUALSENSEDPAD_DOWNLEFT,
    DUALSENSEDPAD_LEFT,
    DUALSENSEDPAD_UPLEFT,
    /* count */
    DUALSENSEDPAD_COUNT,
    } DualSenseDPad;


static DirectXPlayerInput * AsDirectXPlayerInput( Universe* universe );


/*******************************************************************
*
*   PlayerInput_Init()
*
*   DESCRIPTION:
*       Initialize the player input system.
*       Returns TRUE if the GDK library was successfully loaded.
*
*******************************************************************/

bool PlayerInput_Init(Universe* universe)
{    
SingletonPlayerInputComponent* component = (SingletonPlayerInputComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_PLAYER_INPUT, universe);
component->ptr = malloc( sizeof(DirectXPlayerInput) );
if( !component->ptr )
    {
    return( false );
    }

DirectXPlayerInput *input = (DirectXPlayerInput*)component->ptr;
memset( input, 0, sizeof( *input ) );

if (GameInputCreate( &input->game_input_library ) != S_OK)
    {
    return false;
    }
  
  return true;

} /* PlayerInput_Init() */


/*******************************************************************
*
*   PlayerInput_Destroy()
*
*   DESCRIPTION:
*       Destroy the Player Input system and free its resources.
*
*******************************************************************/

void PlayerInput_Destroy(Universe* universe)
{
DirectXPlayerInput *input = AsDirectXPlayerInput( universe );

if (input->the_gamepad ) input->the_gamepad->Release();
if (input->game_input_library ) input->game_input_library->Release();

free( input );
input = NULL;

} /* PlayerInput_Destroy() */


/*******************************************************************
*
*   PlayerInput_DoFrame()
*
*   DESCRIPTION:
*       Advance the system one frame.
*
*******************************************************************/

void PlayerInput_DoFrame( float frame_delta, Universe *universe )
{
//#define MAX_NUM_KEYSTATES    ( 50 )
#define MAX_NUM_CONTROLLER_BUTTONS ( 32 )
#define MAX_NUM_CONTROLLER_AXIS    ( 16 )
#define MAX_NUM_CONTROLLER_SWITCHES    ( 16 )

struct Controller
    {
    uint32_t button_states;

    };

DirectXPlayerInput *input = AsDirectXPlayerInput( universe );
IGameInputReading *reading = NULL;

/* get Keyboard input */
//if (SUCCEEDED(game_input_library->GetCurrentReading(GameInputKindKeyboard, the_keyboard, &reading)))
//    {
//    if (!the_keyboard)
//        {
//        reading->GetDevice(&the_keyboard);
//        }
//
//    uint32_t keyboard_key_count = reading->GetKeyCount();
//    GameInputKeyState state[ MAX_NUM_KEYSTATES ];
//    uint32_t state_count = keyboard_key_count;
//    if (state_count > MAX_NUM_KEYSTATES)
//        {
//        state_count = MAX_NUM_KEYSTATES;
//        }
//
//        // Retrieve the fixed-format gamepad state from the reading.
//        uint32_t num_states_filled_out = reading->GetKeyState(state_count, state);
//        reading->Release();
//        
//        for( uint32_t i = 0; i < num_states_filled_out; i++ )
//            {
//            printf( "Key was pressed: %d\n", state[i].scanCode);
//            }
//    }


/* Get controller input */
if (SUCCEEDED( input->game_input_library->GetCurrentReading(GameInputKindController, input->the_gamepad, &reading)))
    {
    if (!input->the_gamepad) reading->GetDevice( &input->the_gamepad );

     bool button_array[MAX_NUM_CONTROLLER_BUTTONS] = {0};
     float axis_array[MAX_NUM_CONTROLLER_AXIS] = {0};
     GameInputSwitchPosition switch_array[ MAX_NUM_CONTROLLER_SWITCHES ] = {};


    uint32_t button_count = reading->GetControllerButtonCount();
     if (button_count > cnt_of_array(button_array) )
        {
         button_count = cnt_of_array(button_array);
         printf( "WARNING: Too many controller buttons found in PlayerInput system\n");
        }

     uint32_t axis_count = reading->GetControllerAxisCount();
     if (axis_count > cnt_of_array(axis_array) )
     {
         axis_count = cnt_of_array(axis_array);
         printf("WARNING Too many controller axis found in PlayerInput system\n");
     }

    uint32_t switch_count = reading->GetControllerSwitchCount();
    if (switch_count > cnt_of_array(switch_array))
        {
         switch_count = cnt_of_array(switch_array);
         printf("WARNING Too many controller switces found in PlayerInput system\n" );
     }

     uint32_t num_button_states_found =  reading->GetControllerButtonState(button_count, button_array);
     uint32_t  num_axis_states_found = reading->GetControllerAxisState(axis_count, axis_array );
     uint32_t  num_switch_states_found = reading->GetControllerSwitchState( switch_count, switch_array);

    // for (uint32_t i = 0; i < switch_count; i++)
     //   {
     //    printf("%d ", switch_array[i]);

     //   }
    //printf("\n");

     Controller controller = {};
     for(uint32_t i = 0; i < num_button_states_found; i++ )
         {
         if( button_array[ i ] == true )
            {
            controller.button_states |= ( 1 << i );
            }
        }

    // Retrieve the fixed-format gamepad state from the reading.
    //uint32_t num_buttons_filled_out = reading->GetControllerButtonState(button_count,state);

    reading->Release();

        
    /*for( uint32_t i = 0; i < num_buttons_filled_out; i++ )
        {
        if (state[i] == true)
            {
            printf("Hey we got a gamepad button FREEEDOM: %d\n", i);
            }
        }

    }*/
    
    }
} /* PlayerInput_DoFrame() */


/*******************************************************************
*
*   AsDirectXPlayerInput()
*
*   DESCRIPTION:
*       Get the singleton system state.
*
*******************************************************************/

static DirectXPlayerInput * AsDirectXPlayerInput( Universe* universe )
{
SingletonPlayerInputComponent * component = (SingletonPlayerInputComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_PLAYER_INPUT, universe );
return( (DirectXPlayerInput*)component->ptr );

} /* AsDirectXPlayerInput() */
