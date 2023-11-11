#include <GameInput.h>
#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdbool.h>

#include "universe.hpp"
#include "HardwareIDs.hpp"

using namespace ECS;


typedef struct _DirectXPlayerInput
    {
    IGameInput* game_input_library = nullptr;
    IGameInputDevice* the_gamepad = nullptr;
    } DirectXPlayerInput;



static DirectXPlayerInput * AsDirectXPlayerInput( Universe *universe );
static void FindControllerVendorDevice( const GameInputDeviceInfo *controller_info, const size_t num, uint32_t *out_controller_vendor_device, bool *out_is_device_supported );


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

/* get Keyboard input. can consider adding later */
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
    if( is_device_supported = true )
    {
     uint32_t num_button_states_found =  reading->GetControllerButtonState(button_count, button_array);
     uint32_t  num_axis_states_found = reading->GetControllerAxisState(axis_count, axis_array );
     uint32_t  num_switch_states_found = reading->GetControllerSwitchState( switch_count, switch_array);








    }






 //    Controller controller = {};
   //  for(uint32_t i = 0; i < num_button_states_found; i++ )
    //     {
    //     if( button_array[ i ] == true )
     //       {
      //      controller.button_states |= ( 1 << i );
     //       }
      //  }









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

#define set_sony_device( _sonydevice ) \
    ( out_controller_vendor_device[ 1 ] = _sonydevice )

#define set_microsoft_device( _microsoftdevice ) \
    (out_controller_vendor_device[ 1 ] = _microsoftdevice )

#define set_nintendo_device( _nintendodevice ) \
    (out_controller_vendor_device[ 1 ] = _nintendodevice )

#define set_other_device( _otherdevice ) \
    (out_controller_vendor_device[ 1 ] = _otherdevice )

#define supported_device() \
    (*out_is_device_supported = true)

#define unsupported_device() \
    printf( "WARNING Unsupported controller is connected. controller input may not function or may function incorrectly" ); \
    (*out_is_device_supported = false)


ControllerVendor vendor;

switch( controller_info->vendorId )
{
    case 0x54c:  //Sony
        vendor = CONTROLLER_VENDOR_SONY;
        set_vender( vendor );
        SonyControllerDevice sonydevice;
        switch( controller_info->productId )
        {
            case 0x0ce6:
                sonydevice = SONY_CONTROLLER_DEVICE_DUALSENSE;
                set_sony_device( sonydevice );
                supported_device();
            break;

            default:
                sonydevice = SONY_CONTROLLER_DEVICE_OTHER;
                set_sony_device( sonydevice );
                unsupported_device();
         }
    break;

    case 0x045e: //microsoft
        vendor = CONTROLLER_VENDOR_MICROSOFT;
        set_vender( vendor );
        MicrosoftControllerDevice microsoftdevice;
    //    switch( controller_info->productId )   // update here whenever we support microsfot controllers
   //         {
  //          default:
                microsoftdevice = MICROSOFT_CONTROLLER_DEVICE_OTHER;
                set_microsoft_device ( microsoftdevice);
                unsupported_device();
    //        }
    break;

    case 0x057e: //Nintendo
        vendor = CONTROLLER_VENDOR_NINTNEDO;
        set_vender( vendor );
        NintendoControllerDevice nintendodevice;
        switch( controller_info->productId )
    {
        case 2009:
            nintendodevice = NINTENDO_CONTROLLER_DEVICE_SWITCH_PRO;
            set_nintendo_device (nintendodevice);
            supported_device();
        break;

        default:
            nintendodevice = NINTENDO_CONTROLLER_DEVICE_OTHER;
            set_nintendo_device( nintendodevice );
            unsupported_device();
     }
    break;

    default:
        vendor = CONTROLLER_VENDOR_OTHER;
        set_vender( vendor );
        OtherControllerDevice otherdevice;
        otherdevice = OTHER_CONTROLLER_DEVICE_OTHER;
        set_other_device( otherdevice );
        unsupported_device();
}


#undef set_vender
#undef set_sony_device 
#undef set_microsoft_device 
#undef set_nintendo_device 
#undef set_other_device 
#undef supported_device
#undef unsupported_device


} /* FindControllerVendorDevice() */
