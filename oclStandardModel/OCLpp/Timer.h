#ifndef __OCLPP_TIMER_H
#define __OCLPP_TIMER_H

#ifdef WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

namespace ocl {

class Timer {
private:
	double tstart;
	static double getT();
public:
	Timer() { start(); };
	void start();
	double getTime();
};

}
#endif