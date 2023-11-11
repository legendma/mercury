#include <windows.h>
#include <GameInput.h>
#include <cassert>

#include "ApplicationTimer.hpp"
#include "Engine.hpp"


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
HWND hwnd = CreateWindowExW( 0, CLASS_NAME, L"Test Window", WS_OVERLAPPEDWINDOW, 0, 0, 1000, 1000, NULL, NULL, hinstance ,NULL );
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
	default:
		return( DefWindowProc( hwnd, uMsg, wParam, lParam ) );
	}

return( 0 );

} /* WinProc() */
