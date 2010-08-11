#ifndef __OCLPP_IMAGE2D_H
#define __OCLPP_IMAGE2D_H

#include <CL/opencl.h>
#include <iostream>

#include "Buffer.h"
#include "misc.h"

namespace ocl {

class Image2D : public Buffer {
	protected:
		size_t rowPitch;
	public:
		Image2D(cl_mem mem, const size_t rowPitch, cl_command_queue* queue);
		
		void read(void* hostMem, const size_t size[2], const size_t offset[2], cl_bool blocking=CL_TRUE);
		void write(void* hostMem, const size_t size[2], const size_t offset[2], cl_bool blocking=CL_TRUE);
		void* map(cl_map_flags flags, const size_t size[2], const size_t offset[2], size_t& rowPitch, cl_bool blocking=CL_TRUE);

		void copyToBuffer(Buffer& dst, const size_t size[2], const size_t srcOffset[2], const size_t dstOffset=0);
};

}

#endif