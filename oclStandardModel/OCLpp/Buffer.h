#ifndef __OCLPP_BUFFER_H
#define __OCLPP_BUFFER_H

#include <CL/opencl.h>
#include <iostream>
#include <cstdlib>

#include "misc.h"

namespace ocl {

class Image2D;
class Buffer {
protected:
	cl_command_queue* queue;
	cl_mem mem;

public:
	Buffer(cl_mem mem, cl_command_queue* queue);
	~Buffer();

	cl_mem& getMem();

	void read(void* hostMem, const size_t size, const size_t offset=0, const cl_bool blocking=CL_TRUE);
	void write(const void* hostMem, const size_t size, const size_t offset=0, const cl_bool blocking=CL_TRUE);
	void copy(Buffer& dst, const size_t size, const size_t srcOffset=0, const size_t dstOffset=0);
	void* map(const cl_map_flags flags, const size_t size, const size_t offset, const cl_bool blocking=CL_TRUE);
	void unmap(void* mappedPtr);

	void copyToImage2D(Image2D& dst, const size_t size[2], const size_t srcOffset, const size_t dstOffset[2]);
	void copyToImage3D(Image2D& dst, const size_t size[3], const size_t srcOffset, const size_t dstOffset[3]);

	void* getMemInfo(const cl_mem_info paramName);
};


}
#endif
