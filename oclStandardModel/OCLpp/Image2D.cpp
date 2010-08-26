#include "Image2D.h"

using namespace ocl;
using namespace std;

Image2D::Image2D(cl_mem mem, const size_t rowPitch, cl_command_queue* queue) : Buffer(mem, queue) {
	this->rowPitch = rowPitch;
}

void Image2D::read(void* hostMem, const size_t size[2], const size_t offset[2], cl_bool blocking) {
	cl_int err = 0;
	err = clEnqueueReadImage(*queue, mem, blocking, offset, size, rowPitch, 0, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error reading Image2D: " << errorMessage(err) << endl;
		exit(err);
	}
}

void Image2D::write(void* hostMem, const size_t size[2], const size_t offset[2], cl_bool blocking) {
	cl_int err = 0;
	err = clEnqueueWriteImage(*queue, mem, blocking, offset, size, rowPitch, 0, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error writing Image2D: " << errorMessage(err) << endl;
		exit(err);
	}
}

void* Image2D::map(cl_map_flags flags, const size_t size[2], const size_t offset[2], size_t& rowPitch, cl_bool blocking) {
	size_t slicePitch;
	cl_int err = 0;
	void* ret = clEnqueueMapImage(*queue, mem, blocking, flags, offset, size, &rowPitch, &slicePitch, 0, NULL, NULL, &err);
	if (err != CL_SUCCESS) {
		cout << "Error mapping Image2D: " << errorMessage(err) << endl;
		exit(err);
	}
	return ret;
}

void Image2D::copyToBuffer(Buffer& dst, const size_t size[2], const size_t srcOffset[2], const size_t dstOffset) {
	cl_int err = 0;
	err = clEnqueueCopyImageToBuffer(*queue, mem, dst.getMem(), srcOffset, size, dstOffset, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error copying Image2D to Buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

void* Image2D::getInfo(const cl_image_info paramName) {
	cl_int err = 0;
	size_t size;
	err = clGetImageInfo (mem, paramName, 0, NULL, &size);
	if (err != CL_SUCCESS) {
		cout << "Error getting Image info: " << errorMessage(err) << endl;
		exit(err);
	}

	if(size > 0) {
		void* info = malloc(size);
		err = clGetImageInfo (mem, paramName, size, info, &size);
		if (err != CL_SUCCESS) {
			cout << "Error getting Image info: " << errorMessage(err) << endl;
			exit(err);
		}
		return info;
	}
	else return NULL;
}