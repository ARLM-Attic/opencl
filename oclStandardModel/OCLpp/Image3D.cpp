#include "Image3D.h"

using namespace ocl;
using namespace std;

Image3D::Image3D(cl_mem mem, const size_t rowPitch, const size_t slicePitch, cl_command_queue* queue) : Image2D(mem, rowPitch, queue) {
	this->slicePitch = slicePitch;
}

void Image3D::read(void* hostMem, const size_t size[3], const size_t offset[3], cl_bool blocking) {
	cl_int err = 0;
	err = clEnqueueReadImage(*queue, mem, blocking, offset, size, rowPitch, slicePitch, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error reading Image3D: " << errorMessage(err) << endl;
		exit(err);
	}
}

void Image3D::write(void* hostMem, const size_t size[3], const size_t offset[3], cl_bool blocking) {
	cl_int err = 0;
	err = clEnqueueWriteImage(*queue, mem, blocking, offset, size, rowPitch, slicePitch, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error writing Image3D: " << errorMessage(err) << endl;
		exit(err);
	}
}

void* Image3D::map(cl_map_flags flags, const size_t size[3], const size_t offset[3], size_t& rowPitch, size_t& slicePitch, cl_bool blocking) {
	cl_int err = 0;
	void* ret = clEnqueueMapImage(*queue, mem, blocking, flags, offset, size, &rowPitch, &slicePitch, 0, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		cout << "Error mapping Image3D: " << errorMessage(err) << endl;
		exit(err);
	}
	return ret;
}

void Image3D::copyToBuffer(Buffer& dst, const size_t size[3], const size_t srcOffset[3], const size_t dstOffset) {
	cl_int err = 0;
	err = clEnqueueCopyImageToBuffer(*queue, mem, dst.getMem(), srcOffset, size, dstOffset, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error copying Image3D to Buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}