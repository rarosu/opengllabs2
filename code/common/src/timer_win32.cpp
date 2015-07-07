#include "../include/common/timer.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

Timer::Timer()
{
	QueryPerformanceFrequency(reinterpret_cast<LARGE_INTEGER*>(&frequency));
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start));
}

void Timer::Start()
{
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&start));
}

int64_t Timer::End()
{
	int64_t end;
	QueryPerformanceCounter(reinterpret_cast<LARGE_INTEGER*>(&end));

	return (end - start) / (frequency / 1000000);
}