#pragma once

#include <cstdint>

class Timer
{
public:
	Timer();

	/*
		Start measuring time until End() is called.
	*/
	void Start();

	/*
		Returns the time measured either since the last Start() or the constructor.

		Time is returned in microseconds.
	*/
	int64_t End();
private:
	int64_t frequency;
	int64_t start;
};