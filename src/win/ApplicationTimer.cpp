#include <windows.h>

#include "ApplicationTimer.hpp"

void ApplicationTimer_Init(ApplicationTimer *timer)
{
timer->frequency = 0;
timer->seconds_per_count = 0.0;
timer->current_time= 0;
timer->previous_time=0;
timer->frame_delta=0;

QueryPerformanceCounter((LARGE_INTEGER*)&timer->current_time);

QueryPerformanceFrequency((LARGE_INTEGER*) &timer->frequency);

timer->seconds_per_count = 1.0/(double) timer->frequency;

}

void ApplicationTimer_GetDeltaTime(ApplicationTimer *timer)
{
timer->previous_time =timer->current_time;
QueryPerformanceCounter((LARGE_INTEGER*)&timer->current_time);

if (timer->previous_time > timer->current_time)
	{
	timer->previous_time=timer->current_time;
	}

timer->frame_delta = timer->seconds_per_count * (double)(timer->current_time - timer->previous_time);


}

