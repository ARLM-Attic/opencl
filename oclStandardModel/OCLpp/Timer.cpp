#include "Timer.h"
using namespace ocl;

double Timer::getT()
{
#ifdef WIN32
	LARGE_INTEGER c,f;
	QueryPerformanceFrequency(&f);
	QueryPerformanceCounter(&c);
	return (double)c.QuadPart/(double)f.QuadPart;
#else
	struct timeval tv;
	gettimeofday(&tv,0);
	return (double)tv.tv_sec+1.0e-6*(double)tv.tv_usec;
#endif
}

void Timer::start() {
	tstart = getT();
}

double Timer::getTime() {
	return getT() - tstart;
}