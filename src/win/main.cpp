#include <windows.h>
#include <GameInput.h>
#include <cassert>

#include "Universe.hpp"
#include "ApplicationTimer.hpp"
#include "Engine.hpp"
#include "PlayerInput.hpp"

using namespace ECS;
HWND main_window;
bool should_quit;
static const wchar_t *CLASS_NAME = L"Mercury";
bool CreateMainWindow(HINSTANCE hinstance);
LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


int WINAPI main(HINSTANCE hinstance, HINSTANCE previous, PWSTR comandline, int show )
{
should_quit = false;

/* Create the application window */
bool main_window_was_created = CreateMainWindow(hinstance);
if (main_window_was_created == false ) 
	{
	return (-1);
	}



/* TODO - Start the graphic driver */


if (Engine_Init() == false)
{
	return (-1);
}


/* Create the system timer */

ApplicationTimer main_loop_timer;

ApplicationTimer_Init(&main_loop_timer);



/* Main loop */
MSG msg;
while( should_quit == false )
	{
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
		/* Our window had a Window's message in it's queue so let's look at it */
		if (msg.message == WM_QUIT)
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
		ApplicationTimer_GetDeltaTime(&main_loop_timer);
		Engine_DoFrame( (float)main_loop_timer.frame_delta );
		}

	}

/* engine shutdown and cleanup */
Engine_CleanUp();

}


bool CreateMainWindow(HINSTANCE hinstance)
{
/* Register the application class with windows */
WNDCLASS wc = {};
wc.lpfnWndProc = WinProc;
wc.hInstance   = hinstance;
wc.lpszClassName = CLASS_NAME;
RegisterClass(&wc);

/* create the actual window */
main_window = CreateWindowExW(0, CLASS_NAME,L"Test Window",WS_OVERLAPPEDWINDOW,0,0,1000,1000,NULL,NULL,hinstance,NULL);
if (main_window == NULL) 
	{
	return (false);
	}

ShowWindow(main_window,SW_SHOW);

return (true);
}



LRESULT CALLBACK WinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
switch(uMsg)
	{
	case WM_DESTROY:
	    PostQuitMessage( 0 );
		break;
	default:
		return( DefWindowProc(hwnd, uMsg, wParam, lParam) );
	}
return 0;
}


