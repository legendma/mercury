#include <GameInput.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdbool.h>

#include "universe.hpp"
#include "HardwareIDs.hpp"
#include "Utilities.hpp"
#include "math.hpp"

using namespace ECS;


typedef struct _DirectXPlayerInput
    {
    IGameInput* game_input_library = nullptr;
    IGameInputDevice* the_gamepad = nullptr;
    } DirectXPlayerInput;



static DirectXPlayerInput * AsDirectXPlayerInput( Universe *universe );
static void FindControllerVendorDevice( const GameInputDeviceInfo *controller_info, const size_t num, uint32_t *out_controller_vendor_device, bool *out_is_device_supported );
void SetSonyDualsenseButtonPressed( const ControllerSonyDualsense button_in, SingletonControllerInputButtonStateBitArray *ba );


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
#define MAX_NUM_CONTROLLER_AXIS    ( 16 )
#define MAX_NUM_CONTROLLER_SWITCHES    ( 16 )

struct Controller
    {
    uint32_t button_states;

    };

DirectXPlayerInput *input = AsDirectXPlayerInput( universe );
IGameInputReading *reading = NULL;

/* TODO get Keyboard input. can consider adding later */
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
    if (!input->the_gamepad) reading->GetDevice( &input->the_gamepad );   //dynamically add a controller if it is disconnected


    // ensure we don't get massive readings if there are errors in the data stream
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
        uint32_t  num_axis_states_found = reading->GetControllerAxisState( axis_count, axis_array );
        uint32_t  num_switch_states_found = reading->GetControllerSwitchState( switch_count, switch_array );

        /* grab controller input component*/
        SingletonControllerInputComponent *component = (SingletonControllerInputComponent *)Universe_GetSingletonComponent( COMPONENT_SINGLETON_CONTROLLER_INPUT, universe );
        *component = {};

        /* interpret controller data and save it to component */
        switch( vendor_and_device )
            {
            case vendor_and_device_as_u32( CONTROLLER_VENDOR_SONY, CONTROLLER_DEVICE_SONY_DUALSENSE ):
                for( int i = 0; i < CONTROLLER_SONY_DUALSENSE_BUTTONS_LAST; i++ )
                    {
                    if( button_array[i] == false )
                        {
                        continue;
                        }

                    SetSonyDualsenseButtonPressed( (ControllerSonyDualsense)i, &component->button_state );
                    }
                break;

            default:
                printf(" WARNING This controller has been added to the supported device list but the data translation to the compoenent has not been set up yet. ");
                break;
            }
      
        }

    reading->Release();
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
*       Set the generic controller butter for the given Sony
*       Dualsense button id.
*
*******************************************************************/

void SetSonyDualsenseButtonPressed( const ControllerSonyDualsense button_in, SingletonControllerInputButtonStateBitArray *ba )
{
switch( button_in )
    {
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_SQUARE_BUTTOM:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_LEFT_BUTTON );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_X_BUTTON:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_BOT_BUTTON );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_CIRCLE_BUTTON:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_RIGHT_BUTTON );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_TRIANGLE_BUTTON:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_TOP_BUTTON );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_L1_TRIGGER:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_L1 );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_R1_TRIGGER:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_R1 );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_L2_TRIGGER:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_L2);
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_R2_TRIGGER:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_R2 );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_OPTION:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_OPTIONS );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_START:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_MENU );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_LEFT_STICK_CLICK:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_LEFT_STICK_CLICK );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_RIGHT_STICK_CLICK:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_RIGHT_STICK_CLICK );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_POWER_BUTTON:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_POWER );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_CENTER_BUTTON:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_CENTER_PAD );
        break;
    case CONTROLLER_SONY_DUALSENSE_BUTTONS_MUTE_BUTTON:
        Math_BitArraySet( ba->ba, CONTROLLER_BUTTON_MUTE );
        break;

    default:
        //TODO debug_assert( false );
        break;
    }

} /* SetSonyDualsenseButtonPressed() */


