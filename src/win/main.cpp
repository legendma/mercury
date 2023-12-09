#include <windows.h>
#include <GameInput.h>

#include "ApplicationTimer.hpp"
#include "Engine.hpp"

uint16_t s_window_width;
uint16_t s_window_height;
bool     s_is_window_sizing;
bool     s_is_minimized;
bool     s_is_maximized;
bool     s_is_engine_init;


static HWND CreateMainWindow( HINSTANCE hinstance );
static LRESULT CALLBACK WinProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam );


/*******************************************************************
*
*   main()
*
*   DESCRIPTION:
*       Program main entry point.
*
*******************************************************************/

int WINAPI main( HINSTANCE hinstance, HINSTANCE previous, PWSTR comandline, int show )
{
s_window_width = 0;
s_window_height = 0;
s_is_window_sizing = false;
s_is_engine_init   = false;

/* Create the application window */
HWND main_window = CreateMainWindow( hinstance );
if( main_window == NULL ) 
	{
	return( -1 );
	}

/* Initialize the game engine */
if( Engine_Init() == false )
	{
	return (-1);
	}

s_is_engine_init = true;

/* Create the system timer */
ApplicationTimer main_loop_timer;
ApplicationTimer_Init( &main_loop_timer );

/* Run the main loop until the game ends */
MSG msg = {};
bool should_quit = false;
while( should_quit == false )
	{
	if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
		if( msg.message == WM_QUIT )
			{
			should_quit = true;
			}
		else
			{
			TranslateMessage( &msg );
			DispatchMessage( &msg );
			}
		}
	else
		{
		ApplicationTimer_GetDeltaTime( &main_loop_timer );
		Engine_DoFrame( (float)main_loop_timer.frame_delta );
		}

	}

/* Destroy the game engine */
Engine_Destroy();

} /* main() */


/*******************************************************************
*
*   CreateMainWindow()
*
*   DESCRIPTION:
*       Create the MS Windows window.
*
*******************************************************************/

static HWND CreateMainWindow( HINSTANCE hinstance )
{
static const wchar_t *CLASS_NAME = L"Mercury";

/* Register the application class with windows */
WNDCLASS wc = {};
wc.lpfnWndProc = WinProc;
wc.hInstance   = hinstance;
wc.lpszClassName = CLASS_NAME;
RegisterClass(&wc);

/* create the actual window */
HWND hwnd = CreateWindowExW( 0, CLASS_NAME, L"Mercury Engine", ( WS_OVERLAPPED | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX ), 0, 0, 1000, 1000, NULL, NULL, hinstance, NULL );
if( hwnd == NULL )
	{
	return( NULL );
	}

ShowWindow( hwnd, SW_SHOW );

return( hwnd );

} /* CreateMainWindow() */


/*******************************************************************
*
*   WinProc()
*
*   DESCRIPTION:
*       Window message handler.
*
*******************************************************************/

static LRESULT CALLBACK WinProc( HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
switch( uMsg )
	{
	case WM_DESTROY:
	    PostQuitMessage( 0 );
		break;

	case WM_SIZE:
		// Save the new client area dimensions.
		s_window_width = (uint16_t)LOWORD(lParam);
		s_window_height = (uint16_t)HIWORD(lParam);
		if( s_is_engine_init )
			{
			/*if( wParam == SIZE_MINIMIZED )
				{
				mAppPaused = true;
				mMinimized = true;
				mMaximized = false;
				}
			else*/ if( wParam == SIZE_MAXIMIZED )
				{
				//mAppPaused = false;
				s_is_minimized = false;
				s_is_maximized = true;
				Engine_ChangeResolutions( s_window_width, s_window_height );
				}
			else if( wParam == SIZE_RESTORED )
				{
				
				// Restoring from minimized state?
				if( s_is_minimized )
					{
					//mAppPaused = false;
					s_is_minimized = false;
					Engine_ChangeResolutions( s_window_width, s_window_height );
					}

				// Restoring from maximized state?
				else if( s_is_maximized )
					{
					//mAppPaused = false;
					s_is_maximized = false;
					Engine_ChangeResolutions( s_window_width, s_window_height );
					}
				else if( s_is_window_sizing )
					{
					// If user is dragging the resize bars, we do not resize 
					// the buffers here because as the user continuously 
					// drags the resize bars, a stream of WM_SIZE messages are
					// sent to the window, and it would be pointless (and slow)
					// to resize for each WM_SIZE message received from dragging
					// the resize bars.  So instead, we reset after the user is 
					// done resizing the window and releases the resize bars, which 
					// sends a WM_EXITSIZEMOVE message.
					}
				else // API call such as SetWindowPos or SwapChain->SetFullscreenState.
					{
					Engine_ChangeResolutions( s_window_width, s_window_height );
					}
				}
			}
		break;

	// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
	case WM_ENTERSIZEMOVE:
		//mAppPaused = true;
		s_is_window_sizing = true;
		//mTimer.Stop();
		break;

	// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
	// Here we reset everything based on the new window dimensions.
	case WM_EXITSIZEMOVE:
		//mAppPaused = false;
		s_is_window_sizing = false;
		//mTimer.Start();
		Engine_ChangeResolutions( s_window_width, s_window_height );
		break;

	default:
		return( DefWindowProc( hwnd, uMsg, wParam, lParam ) );
	}

return( 0 );

} /* WinProc() */
