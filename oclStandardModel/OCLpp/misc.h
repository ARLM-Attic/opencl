#ifndef __OCLPP_MISC_H
#define __OCLPP_MISC_H

#include <CL/cl.h>
#include <string>
#include <cstdio>
#include <cstdlib>

std::string errorMessage(const int error);
char* loadProgSource(const char* filename, size_t* finalLength);

#endif
