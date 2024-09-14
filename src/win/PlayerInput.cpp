#include <GameInput.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdbool.h>


#include "ControllerInputUtilities.hpp"
#include "Event.hpp"
#include "HardwareIDs.hpp"
#include "math.hpp"
#include "universe.hpp"
#include "Utilities.hpp"


using namespace ECS;

/* NOTE: each suppported controller needs to have an enum reported here in this , seperate list. TODO <jw> use the max_of_vals macro in front of this enum list once we have more than one controller supported */
#define MAX_NUM_CONTROLLER_AXIS    ( CONTROLLER_SONY_DUALSENSE_AXIS_COUNT )// max_of_vals
#define MAX_NUM_CONTROLLER_TRIGGERS    ( CONTROLLER_TRIGGERS_FLOAT_DATA_COUNT )// max_of_vals
#define MAX_NUM_CONTROLLER_STICK_AXIS    ( CONTROLLER_STICK_AXIS_FLOAT_DATA_COUNT )// max_of_vals

typedef struct _DirectXPlayerInput
    {
    IGameInput* game_input_library = nullptr;
    IGameInputDevice* the_gamepad = nullptr;
    IGameInputDevice* the_keyboard = nullptr;
    GameInputKeyState keyboard_key_pressed [ KEYBOARD_KEY_COUNT ];
    float controller_trigger_dead_zone[ CONTROLLER_DEVICE_COUNT ];
    float controller_stick_dead_zone [CONTROLLER_DEVICE_COUNT ];
    } DirectXPlayerInput;

typedef struct _AxisReadingArray
    {
    float               arr[ MAX_NUM_CONTROLLER_AXIS ];
    } AxisReadingArray;

typedef struct _TriggerWritingArray
    {
    float               arr[ MAX_NUM_CONTROLLER_TRIGGERS ];
    } TriggerWritingArray;

typedef struct _StickWritingArray
    {
    float              left_stick [MAX_NUM_CONTROLLER_STICK_AXIS];
    float              right_stick[MAX_NUM_CONTROLLER_STICK_AXIS];
    }StickWritingArray;



static DirectXPlayerInput * AsDirectXPlayerInput( Universe *universe );
static void FindControllerVendorDevice( const GameInputDeviceInfo *controller_info, const size_t num, uint32_t *out_controller_vendor_device, bool *out_is_device_supported );
static void SetSonyDualsenseButtonPressed( const ControllerSonyDualsense button_in, SingletonControllerInputButtonStateBitArray *ba );
static void SetSonyDualsenseDpadPressed( GameInputSwitchPosition switch_status, SingletonControllerInputButtonStateBitArray *ba );
static void SonyDualsenseTriggerAndStickSorting( const AxisReadingArray *read_axis_array, TriggerWritingArray *trigger_array, StickWritingArray *stick_array );
static void NormalizeControllerTriggers( TriggerWritingArray *trigger_array, DirectXPlayerInput *trigger_dead_zone_for_controller, const ControllerDevice controller_device_type );
static void NormalizeControllerStickAxis( StickWritingArray *stick_arry, DirectXPlayerInput *stick_dead_zone_for_controller, const ControllerDevice controller_device_type );
static void WriteControllerTriggerAndStickAxisDataToComponent( SingletonControllerInputComponent *componentdata, TriggerWritingArray *trigger_array, StickWritingArray *stick_array );
static void TestControllerComponentData( float frame_delta, SingletonControllerInputComponent *componentdata );






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
*input = {};

if( GameInputCreate( &input->game_input_library ) != S_OK )
    {
    return false;
    }

/* controller dead zone settings. TODO move this to a LUA script */
input->controller_trigger_dead_zone[CONTROLLER_DEVICE_SONY_DUALSENSE] = 0.1f;
input->controller_stick_dead_zone[CONTROLLER_DEVICE_SONY_DUALSENSE] = 0.1f;

SingletonControllerInputComponent *controller = (SingletonControllerInputComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_CONTROLLER_INPUT, universe );
*controller = {};

SingletonKeyboardInputComponent *keyboard = (SingletonKeyboardInputComponent *)Universe_GetSingletonComponent(COMPONENT_SINGLETON_KEYBOARD_INPUT, universe );
*keyboard = {};

/* setup the utilties functions in the controller input component */
InitializeControllerButtonUtilities( controller );
  
return( true );

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
#define vendor_and_device_as_u32( _vendor, _device ) \
    ( ( (_vendor) << 16 ) | ( 0xffff & _device ) )

//#define MAX_NUM_KEYSTATES    ( 50 )
#define MAX_NUM_CONTROLLER_BUTTONS ( 32 )
#define MAX_NUM_CONTROLLER_SWITCHES    ( 16 )  

struct Controller
    {
    uint32_t button_states;
    };

DirectXPlayerInput *input = AsDirectXPlayerInput( universe );
IGameInputReading *reading = NULL;


/* Get controller input */
if (SUCCEEDED( input->game_input_library->GetCurrentReading(GameInputKindController, input->the_gamepad, &reading)))
    {
    if (!input->the_gamepad) reading->GetDevice( &input->the_gamepad );   //dynamically add a controller if it is disconnected


    // initialize controller input variables
     bool button_array[MAX_NUM_CONTROLLER_BUTTONS] = { 0 };
     AxisReadingArray read_axis_array = {};
     TriggerWritingArray write_triggers_array = {};
     GameInputSwitchPosition switch_array[ MAX_NUM_CONTROLLER_SWITCHES ] = {}; 
     StickWritingArray write_stick_axis_array = {};

     // check how many inputs we have for the connected controller
    uint32_t button_count = reading->GetControllerButtonCount();
    uint32_t axis_count = reading->GetControllerAxisCount();
    uint32_t switch_count = reading->GetControllerSwitchCount();


    // ensure we don't get massive readings if there are errors in the data stream
     if (button_count > MAX_NUM_CONTROLLER_BUTTONS )
        {
         button_count = MAX_NUM_CONTROLLER_BUTTONS;
         printf( "WARNING: Too many controller buttons found in PlayerInput system\n");
     }

     if (axis_count > MAX_NUM_CONTROLLER_AXIS )
     {
         axis_count = MAX_NUM_CONTROLLER_AXIS;
         printf("WARNING Too many controller axis found in PlayerInput system\n");
     }

    if (switch_count > MAX_NUM_CONTROLLER_SWITCHES )
        {
         switch_count = MAX_NUM_CONTROLLER_SWITCHES;
         printf("WARNING Too many controller switces found in PlayerInput system\n" );
     }

     /* pull what kind of controller is connected  */
    const GameInputDeviceInfo *controller_info = NULL;
    controller_info = input->the_gamepad->GetDeviceInfo();
    uint32_t controller_vendor_device[ 2 ]={};
    bool is_device_supported = false;

    FindControllerVendorDevice( controller_info, cnt_of_array( controller_vendor_device ), controller_vendor_device, &is_device_supported );

    // get raw data from the controller. don't collect data if the controller is unsupported
    if( is_device_supported == true )
        {
        compiler_assert( CONTROLLER_DEVICE_COUNT < max_uint_value( uint16_t ), player_input_cpp );
        compiler_assert( CONTROLLER_VENDOR_COUNT < max_uint_value( uint16_t ), player_input_cpp );
        uint32_t vendor_and_device = vendor_and_device_as_u32( controller_vendor_device[ 0 ], controller_vendor_device[ 1 ] );

        /* grab controller data */
        uint32_t num_button_states_found = reading->GetControllerButtonState( button_count, button_array );
        uint32_t  num_axis_states_found = reading->GetControllerAxisState( axis_count, read_axis_array.arr );
        uint32_t  num_switch_states_found = reading->GetControllerSwitchState( switch_count, switch_array );

        printf("controller found %d", num_button_states_found );

        /* grab controller input component*/
        SingletonControllerInputComponent *component = (SingletonControllerInputComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_CONTROLLER_INPUT, universe );
        component->button_state = {0};
        //component->axis_state = {0};
        //component->trigger_state = { 0 };

        /* interpret controller data and save it to component */
        switch( vendor_and_device )
            {
            case vendor_and_device_as_u32( CONTROLLER_VENDOR_SONY, CONTROLLER_DEVICE_SONY_DUALSENSE ):
                for( uint32_t i = 0; i < button_count; i++ )
                    {
                    if( button_array[i] == false )
                        {
                        continue;
                        }
                    SetSonyDualsenseButtonPressed( (ControllerSonyDualsense)(i + CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST), &component->button_state );
                    }
                for( uint32_t j = 0; j < switch_count; j++ )
                    {
                    SetSonyDualsenseDpadPressed( switch_array [ j ], &component->button_state);
                    }
                /* assign sony_dualsense controller Trigger and stick Axis float data to the generic trigger and stick strutures */
                SonyDualsenseTriggerAndStickSorting(&read_axis_array, &write_triggers_array, &write_stick_axis_array );

                /* apply dead zone setting to trigger data. any input < dead zone = 0 */
                NormalizeControllerTriggers( &write_triggers_array, input, CONTROLLER_DEVICE_SONY_DUALSENSE );

                /* normalize controller stick axis data and apply dead zone */
                NormalizeControllerStickAxis( &write_stick_axis_array, input, CONTROLLER_DEVICE_SONY_DUALSENSE );

                /* Write Trigger and Axis Array data to component */
                WriteControllerTriggerAndStickAxisDataToComponent( component, &write_triggers_array, &write_stick_axis_array );

                break;

            default:
                printf(" WARNING This controller has been added to the supported device list but the data translation to the compoenent has not been set up yet. ");
                break;
            }
        /* print out the controller component data for debuging purposes */
        //TestControllerComponentData( frame_delta, component );
        }
    reading->Release();
    }

/* Get Keyboad input */
 if( SUCCEEDED( input->game_input_library->GetCurrentReading( GameInputKindKeyboard, input->the_keyboard,&reading ) ) )
    {
    if( !input->the_keyboard ) reading->GetDevice( &input->the_keyboard );

    SingletonKeyboardInputComponent *component = (SingletonKeyboardInputComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_KEYBOARD_INPUT, universe );

    // save the prior state and zero out current keyboard key state
    SingletonKeyboardKeyBitArray prev_keyboard_key_pressed = component->keyboard_key_pressed;
    clr_array( component->keyboard_key_pressed.ba );

    component->number_of_keys_pressed = 0; 
    component->number_of_keys_pressed = (u8) reading->GetKeyCount();

    if( component->number_of_keys_pressed > 0 ) //only update if there is a key that needs pressed
        {
            uint32_t dummy = reading->GetKeyState( component->number_of_keys_pressed, input->keyboard_key_pressed );
       
        //throw out a key if it outputs an un-supported scan code
        for( uint32_t i = 0; i < component->number_of_keys_pressed; i++ )
        {
            if( (uint32_t)input->keyboard_key_pressed[i].scanCode > (KEYBOARD_KEY_COUNT - 1) )
            {
                input->keyboard_key_pressed[i].scanCode = 0;
                component->number_of_keys_pressed = component->number_of_keys_pressed - 1;
            }
            //assign current key to component
            Math_BitArraySet(component->keyboard_key_pressed.ba, (uint32_t)input->keyboard_key_pressed[i].scanCode );
        }
    }

    for( int i = 0; i < cnt_of_array( component->keyboard_key_changed.ba ); i++ )
        {
        component->keyboard_key_changed.ba[ i ] = component->keyboard_key_pressed.ba[ i ] ^ prev_keyboard_key_pressed.ba[ i ];
        }

 }

    
/* Get Mouse input */

} /* PlayerInput_DoFrame() */



/*******************************************************************
*
*   AsDirectXPlayerInput()
*
*   DESCRIPTION:
*       Get the singleton system state.
*
*******************************************************************/

static DirectXPlayerInput * AsDirectXPlayerInput( Universe *universe )
{
SingletonPlayerInputComponent * component = (SingletonPlayerInputComponent*)Universe_GetSingletonComponent( COMPONENT_SINGLETON_PLAYER_INPUT, universe );
return( (DirectXPlayerInput*)component->ptr );

} /* AsDirectXPlayerInput() */


/*******************************************************************
*
*   FindControllerVendorDevice()
*
*   DESCRIPTION:
*       get the vendor and device ID of the current controller. 
*       will print out a warning if the controller is unsupported
*
*******************************************************************/

static void FindControllerVendorDevice( const GameInputDeviceInfo *controller_info, const size_t num, uint32_t *out_controller_vendor_device, bool *out_is_device_supported )
{
assert( num == 2 );

#define set_vender( _vendor ) \
   ( out_controller_vendor_device[ 0 ] = _vendor )

#define set_device( _device ) \
    ( out_controller_vendor_device[ 1 ] = _device )

#define supported_device() \
    (*out_is_device_supported = true)

#define unsupported_device() \
    printf( "WARNING Unsupported controller is connected. controller input may not function or may function incorrectly" ); \
    (*out_is_device_supported = false)


ControllerVendor vendor;
ControllerDevice device;

switch( controller_info->vendorId )
{
    case 0x54c:  //Sony
        vendor = CONTROLLER_VENDOR_SONY;
        set_vender( vendor );
        switch( controller_info->productId )
        {
            case 0x0ce6:
                device = CONTROLLER_DEVICE_SONY_DUALSENSE;
                supported_device();
            break;

            default:
                device = CONTROLLER_DEVICE_SONY_OTHER;
                set_device( device );
                unsupported_device();
         }
    break;

    case 0x045e: //microsoft
        vendor = CONTROLLER_VENDOR_MICROSOFT;
        set_vender( vendor );
    //    switch( controller_info->productId )   // TODO update here whenever we support microsfot controllers
   //         {
  //          default:
                device = CONTROLLER_DEVICE_MICROSOFT_OTHER;
                set_device ( device);
                unsupported_device();
    //        }
    break;

    case 0x057e: //Nintendo
        vendor = CONTROLLER_VENDOR_NINTENDO;
        set_vender( vendor );
        switch( controller_info->productId )
    {
        case 2009:
            device = CONTROLLER_DEVICE_NINTENDO_SWITCH_PRO;
            set_device (device);
            supported_device();
        break;

        default:
            device = CONTROLLER_DEVICE_NINTENDO_OTHER;
            set_device( device );
            unsupported_device();
     }
    break;

    default:
        vendor = CONTROLLER_VENDOR_OTHER;
        set_vender( vendor );
        device = CONTROLLER_DEVICE_OTHER_OTHER;
        set_device( device );
        unsupported_device();
}


#undef set_vender
#undef set_device 
#undef supported_device
#undef unsupported_device


} /* FindControllerVendorDevice() */


/*******************************************************************
*
*   SetSonyDualsenseButtonPressed()
*
*   DESCRIPTION:
*       Set the generic controller butten for the given Sony
*       Dualsense button id.
*
*******************************************************************/

static void SetSonyDualsenseButtonPressed( const ControllerSonyDualsense button_in, SingletonControllerInputButtonStateBitArray *ba )
{
switch( button_in )
    {
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_SQUARE_BUTTOM - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST) :
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_LEFT_BUTTON );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_X_BUTTON - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_BOT_BUTTON );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_CIRCLE_BUTTON - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_RIGHT_BUTTON );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_TRIANGLE_BUTTON - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_TOP_BUTTON );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_L1_TRIGGER - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_L1 );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_R1_TRIGGER - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_R1 );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_L2_TRIGGER - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_L2);
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_R2_TRIGGER - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_R2 );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_OPTION - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_OPTIONS );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_START - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_MENU );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_LEFT_STICK_CLICK - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_LEFT_STICK_CLICK );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_RIGHT_STICK_CLICK - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_RIGHT_STICK_CLICK );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_POWER_BUTTON - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_POWER );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_CENTER_BUTTON - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_CENTER_PAD );
        break;
    case (CONTROLLER_SONY_DUALSENSE_BUTTONS_MUTE_BUTTON - CONTROLLER_SONY_DUALSENSE_BUTTONS_FIRST):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_MUTE );
        break;

    default:
        debug_assert( false );
        break;
    }

} /* SetSonyDualsenseButtonPressed() */


/*******************************************************************
*
*   SetSonyDualsenseDpadPressed()
*
*   DESCRIPTION:
*       Set the generic D-pad butten for the given Sony
*       Dualsense button id.
*
*******************************************************************/

static void SetSonyDualsenseDpadPressed( GameInputSwitchPosition switch_status, SingletonControllerInputButtonStateBitArray *ba )
{
    switch( switch_status )
    {
    case( GameInputSwitchUp ):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_UP );
        break;
    case( GameInputSwitchUpRight ):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_UPRIGHT );
        break;
    case( GameInputSwitchRight ):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_RIGHT );
        break;
    case( GameInputSwitchDownRight ):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_DOWNRIGHT );
        break;
    case( GameInputSwitchDown) :
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_DOWN );
        break;
    case( GameInputSwitchDownLeft ):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_DOWNLEFT );
        break;
    case( GameInputSwitchLeft ):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_LEFT );
        break;
    case( GameInputSwitchUpLeft ):
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_UPLEFT );
        break;

    default:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_DPAD_CENTER );
        
        break;
    }

} /* SetSonyDualsenseDpadPressed() */



/*******************************************************************
*
*   SonyDualsenseTriggerAndStickSorting()
*
*   DESCRIPTION:
*       Seperates out the raw axis data into trigger and axis
*       specific arrays for the sony _ualsense controller
*
*******************************************************************/

static void SonyDualsenseTriggerAndStickSorting( const AxisReadingArray *read_axis_array, TriggerWritingArray *trigger_array, StickWritingArray *stick_array )
{
    trigger_array->arr[CONTROLLER_TRIGGERS_FLOAT_DATA_LEFT_TRIGGER] = read_axis_array->arr[ CONTROLLER_SONY_DUALSENSE_AXIS_LEFT_TRIGGER  - CONTROLLER_SONY_DUALSENSE_AXIS_FIRST ];
    trigger_array->arr[CONTROLLER_TRIGGERS_FLOAT_DATA_RIGHT_TRIGGER] = read_axis_array->arr[ CONTROLLER_SONY_DUALSENSE_AXIS_RIGHT_TRIGGER - CONTROLLER_SONY_DUALSENSE_AXIS_FIRST ];

    stick_array->left_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_X] = read_axis_array->arr[CONTROLLER_SONY_DUALSENSE_AXIS_LEFT_STICK_X - CONTROLLER_SONY_DUALSENSE_AXIS_FIRST];
    stick_array->left_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_Y] = read_axis_array->arr[CONTROLLER_SONY_DUALSENSE_AXIS_LEFT_STICK_Y - CONTROLLER_SONY_DUALSENSE_AXIS_FIRST];

    stick_array->right_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_X] = read_axis_array->arr[CONTROLLER_SONY_DUALSENSE_AXIS_RIGHT_STICK_X - CONTROLLER_SONY_DUALSENSE_AXIS_FIRST];
    stick_array->right_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_Y] = read_axis_array->arr[CONTROLLER_SONY_DUALSENSE_AXIS_RIGHT_STICK_Y - CONTROLLER_SONY_DUALSENSE_AXIS_FIRST] ;

} /* SonyDualsenseTriggerAndStickSorting() */


/*******************************************************************
*
*   NormalizeControllerTriggers()
*
*   DESCRIPTION:
*       Apply dead zones and normalize the controller Triggers
*
*******************************************************************/

static void NormalizeControllerTriggers( TriggerWritingArray *trigger_array, DirectXPlayerInput *trigger_dead_zone_for_controller,  const ControllerDevice controller_device_type )
{
    for( int i = 0; i < CONTROLLER_TRIGGERS_FLOAT_DATA_COUNT; i++ )
    {
        if( trigger_array->arr[i] < trigger_dead_zone_for_controller->controller_trigger_dead_zone[ controller_device_type ] )
        {
            trigger_array->arr[ i ] = 0.0;
        }

    }
} /* NormalizeControllerTriggers() */


/*******************************************************************
*
*   NormalizeControllerStickAxis()
*
*   DESCRIPTION:
*       Apply dead zones and normalize the controller stick x and y axis
*
*******************************************************************/

static void NormalizeControllerStickAxis( StickWritingArray *stick_arry, DirectXPlayerInput *stick_dead_zone_for_controller, const ControllerDevice controller_device_type )
{

    for( int i = 0; i < CONTROLLER_STICK_AXIS_FLOAT_DATA_COUNT; i++ )
    {
        /* normalize data from 0-1 to -1-1 */
        stick_arry->left_stick[ i ] = 2 * stick_arry->left_stick[ i ] - 1;
        stick_arry->right_stick[ i ] = 2 * stick_arry->right_stick[ i ] - 1;

        /* apply dead zone to squash data near zero */
        if( abs( stick_arry->left_stick[ i ] ) < stick_dead_zone_for_controller->controller_stick_dead_zone[controller_device_type] )
        {
            stick_arry->left_stick[ i ] = 0.0;
        }

        if( abs( stick_arry->right_stick[ i ] ) < stick_dead_zone_for_controller->controller_stick_dead_zone[controller_device_type] )
        {
            stick_arry->right_stick[ i ] = 0.0;
        }
    }
} /* NormalizeControllerStickAxis() */


/*******************************************************************
*
*   WriteControllerTriggerAndStickAxisDataToComponent()
*
*   DESCRIPTION:
*       Apply dead zones and normalize the controller stick x and y axis
*
*******************************************************************/

static void WriteControllerTriggerAndStickAxisDataToComponent( SingletonControllerInputComponent *componentdata, TriggerWritingArray *trigger_array, StickWritingArray *stick_array )
{
    /* write trigger data*/
    for( int i = 0; i < CONTROLLER_TRIGGERS_FLOAT_DATA_COUNT; i++ )
    {
        componentdata->trigger_state[ i ] = trigger_array->arr[i];
    }

    /* write stick data */
    Float2 left_stick_vector = {0};
    Float2 right_stick_vector = {0};
    
    left_stick_vector.v.x = stick_array->left_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_X];
    left_stick_vector.v.y = stick_array->left_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_Y];
    right_stick_vector.v.x = stick_array->right_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_X];
    right_stick_vector.v.y = stick_array->right_stick[CONTROLLER_STICK_AXIS_FLOAT_DATA_Y];

    componentdata->axis_state[ CONTROLLER_AXIS_LEFT_STICK ] = left_stick_vector;
    componentdata->axis_state[ CONTROLLER_AXIS_RIGHT_STICK ] = right_stick_vector;

} /* WriteControllerTriggerAndStickAxisDataToComponent() */



/*******************************************************************
*
*   TestControllerComponentData()
*
*   DESCRIPTION:
*       Apply dead zones and normalize the controller stick x and y axis
*
*******************************************************************/

static void TestControllerComponentData( float frame_delta, SingletonControllerInputComponent *componentdata )
{
static float clock;
typedef struct _ButtonNamePair
    {
        ControllerButtons    key;
        const char*          name;
    }ButtonNamePair;

static const ButtonNamePair BUTTON_KEY_NAMES[] =
    {
    { CONTROLLER_BUTTON_DPAD_CENTER,        "DPAD_CENTER"          },
    { CONTROLLER_BUTTON_DPAD_UP,            "DPAD_UP"              },
    { CONTROLLER_BUTTON_DPAD_UPRIGHT,       "DPAD_UPRIGHT"         },
    { CONTROLLER_BUTTON_DPAD_RIGHT,         "DPAD_RIGHT"           },
    { CONTROLLER_BUTTON_DPAD_DOWNRIGHT,     "DPAD_DOWNRIGHT"       },
    { CONTROLLER_BUTTON_DPAD_DOWN,          "DPAD_DOWN"            },
    { CONTROLLER_BUTTON_DPAD_DOWNLEFT,      "DPAD_DOWNLEFT"        },
    { CONTROLLER_BUTTON_DPAD_LEFT,          "DPAD_LEFT"            },
    { CONTROLLER_BUTTON_DPAD_UPLEFT,        "DPAD_UPLEFT,"         },
    { CONTROLLER_BUTTON_LEFT_STICK_CLICK,   "LEFT_STICK_CLICK"     },
    { CONTROLLER_BUTTON_RIGHT_STICK_CLICK,  "RIGHT_STICK_CLICK"    },
    { CONTROLLER_BUTTON_L1,                 "L1"                   },
    { CONTROLLER_BUTTON_R1,                 "R1"                   },
    { CONTROLLER_BUTTON_L2,                 "L2"                   },
    { CONTROLLER_BUTTON_R2,                 "R2"                   },
    { CONTROLLER_BUTTON_MENU,               "MENU"                 },
    { CONTROLLER_BUTTON_OPTIONS,            "OPTIONS"              },
    { CONTROLLER_BUTTON_CENTER_PAD,         "CENTER_PAD"           },
    { CONTROLLER_BUTTON_POWER,              "POWER"                },
    { CONTROLLER_BUTTON_MUTE,               "MUTE"                 },
    { CONTROLLER_BUTTON_BOT_BUTTON,         "BOT_BUTTON,"          },
    { CONTROLLER_BUTTON_TOP_BUTTON,         "TOP_BUTTON"           },
    { CONTROLLER_BUTTON_LEFT_BUTTON,        "LEFT_BUTTON"          },
    { CONTROLLER_BUTTON_RIGHT_BUTTON,       "RIGHT_BUTTON"         }
    };

clock += frame_delta;
if( clock < 1.0f )
    {
    return;
    }

clock = 0.0f;

printf( "button state Data \n");
for( int i = 0; i < CONTROLLER_BUTTON_COUNT; i++ )
    {
    if( !Math_BitArrayIsSet( componentdata->button_state.ba, i ) )
        {
        continue;
        }
    
    for( int j = 0; j < cnt_of_array( BUTTON_KEY_NAMES ); j++ )
    {
        if( BUTTON_KEY_NAMES[j].key != i )
        {
            continue;
        }

    printf( "button pressed = %s \n", BUTTON_KEY_NAMES[ j ].name);
    break;
    }
    }

printf( "axis state data \n" );
printf("Left Stick Axis ");
printf("X = %f",componentdata->axis_state[CONTROLLER_AXIS_LEFT_STICK].v.x);
printf( ", Y = %f \n", componentdata->axis_state[CONTROLLER_AXIS_LEFT_STICK].v.y);
printf( "Right Stick Axis " );
printf( "X = %f", componentdata->axis_state[CONTROLLER_AXIS_RIGHT_STICK].v.x );
printf( ", Y = %f \n", componentdata->axis_state[CONTROLLER_AXIS_RIGHT_STICK].v.y );

printf( "Trigger state Data \n");
printf( "Left Trigger = %f, Right Trigger = %f \n\n", componentdata->trigger_state[CONTROLLER_TRIGGERS_LEFT], componentdata->trigger_state[CONTROLLER_TRIGGERS_RIGHT] );

} /* TestControllerComponentData() */



/*******************************************************************
*
*   PlayerInput_IsKeyboardKeyGoingDown()
*
*   DESCRIPTION:
*       Checks if the Keyboard key is going down.
*
*******************************************************************/

bool PlayerInput_IsKeyboardKeyGoingDown( KeyboardKeyScanCode key, Universe *universe )
{
    SingletonKeyboardInputComponent *component = (SingletonKeyboardInputComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_KEYBOARD_INPUT, universe );
   if (!Math_BitArrayIsSet(component->keyboard_key_changed.ba,key)) return false;

   return (Math_BitArrayIsSet( component->keyboard_key_pressed.ba, key ));

} /* PlayerInput_IsKeyboardKeyGoingDown() */


/*******************************************************************
*
*   PlayerInput_IsKeyboardKeyGoingUp()
*
*   DESCRIPTION:
*       Checks if the Keyboard key is going up.
*
*******************************************************************/

bool PlayerInput_IsKeyboardKeyGoingUp( KeyboardKeyScanCode key, Universe *universe )
{
    SingletonKeyboardInputComponent *component = (SingletonKeyboardInputComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_KEYBOARD_INPUT, universe );
   if (!Math_BitArrayIsSet(component->keyboard_key_changed.ba,key)) return false;

   return (!Math_BitArrayIsSet( component->keyboard_key_pressed.ba, key ));

} /* PlayerInput_IsKeyboardKeyGoingUp() */



