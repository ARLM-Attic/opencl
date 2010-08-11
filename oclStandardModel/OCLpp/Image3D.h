#ifndef __OCLPP_IMAGE3D_H
#define __OCLPP_IMAGE3D_H

#include <CL/opencl.h>
#include <iostream>

#include "Image2D.h"
#include "misc.h"

namespace ocl {

class Image3D : public Image2D {
protected:
	size_t slicePitch;
public:
	Image3D(cl_mem mem, const size_t rowPitch, const size_t slicePitch, cl_command_queue* queue);

	void read(void* hostMem, const size_t size[3], const size_t offset[3], cl_bool blocking=CL_TRUE);
	void write(void* hostMem, const size_t size[3], const size_t offset[3], cl_bool blocking=CL_TRUE);
	void* map(cl_map_flags flags, const size_t size[3], const size_t offset[3], size_t& rowPitch, size_t& slicePitch, cl_bool blocking=CL_TRUE);

	void copyToBuffer(Buffer& dst, const size_t size[3], const size_t srcOffset[3], const size_t dstOffset=0);
};

}

#endif