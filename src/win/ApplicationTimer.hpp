#pragma once

struct ApplicationTimer
{
	__int64 frequency;
	double seconds_per_count;
	__int64 current_time;
	__int64 previous_time;
	double frame_delta;
};


void ApplicationTimer_Init(ApplicationTimer* timer);

void ApplicationTimer_GetDeltaTime(ApplicationTimer* timer);