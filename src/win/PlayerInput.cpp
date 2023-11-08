#include <GameInput.h>
#include <cstdio>


IGameInputDevice* the_keyboard = nullptr;
IGameInputDevice* the_gamepad = nullptr;
IGameInput* game_input_library = nullptr;

/*Initialize the gaming input devices (mouse, keyboard, controller, etc). Returns true if the initialization was sucessful.*/
bool PlayerInput_InitializeController()
{    
if (GameInputCreate(&game_input_library) != S_OK)
    {
    return false;
    }
  
  return true;
}

/*Shutdown the game input devices*/
void PlayerInput_ShutdownController()
{
    if (the_gamepad) the_gamepad->Release();
    if (the_keyboard) the_keyboard->Release();
    if (game_input_library) game_input_library->Release();

}


/* TODO - get current input*/

void PlayerInput_GetCurrentInput()
{
#define MAX_NUM_KEYSTATES    ( 50 )
#define MAX_NUM_CONTROLLER_BUTTONS    ( 50 )

IGameInputReading* reading;

/* get Keyboard input */
if (SUCCEEDED(game_input_library->GetCurrentReading(GameInputKindKeyboard, the_keyboard, &reading)))
    {
    if (!the_keyboard)
        {
        reading->GetDevice(&the_keyboard);
        }

    uint32_t keyboard_key_count = reading->GetKeyCount();
    GameInputKeyState state[ MAX_NUM_KEYSTATES ];
    uint32_t state_count = keyboard_key_count;
    if (state_count > MAX_NUM_KEYSTATES)
        {
        state_count = MAX_NUM_KEYSTATES;
        }

        // Retrieve the fixed-format gamepad state from the reading.
        uint32_t num_states_filled_out = reading->GetKeyState(state_count, state);
        reading->Release();
        
        for( uint32_t i = 0; i < num_states_filled_out; i++ )
            {
            printf( "Key was pressed: %d\n", state[i].scanCode);
            }
    }

/* Get Game Pad input */
if (SUCCEEDED(game_input_library->GetCurrentReading(GameInputKindController, the_gamepad, &reading)))
    {
    if (!the_gamepad) reading->GetDevice(&the_gamepad);

    uint32_t controller_button_count = reading->GetControllerButtonCount();
    bool state[MAX_NUM_CONTROLLER_BUTTONS];
    uint32_t button_count = controller_button_count;
    if (button_count > MAX_NUM_CONTROLLER_BUTTONS)
        {
        button_count = MAX_NUM_CONTROLLER_BUTTONS;
      }

    // Retrieve the fixed-format gamepad state from the reading.
    uint32_t num_buttons_filled_out = reading->GetControllerButtonState(button_count,state);
    reading->Release();
        
    for( uint32_t i = 0; i < num_buttons_filled_out; i++ )
        {
        if (state[i] == true)
            {
            printf("Hey we got a gamepad button FREEEDOM: %d\n", i);
            }
        }

    }

}



// get keyboard input



/* TODO -- Get previous input */