#include "Buffer.h"
#include "Image2D.h"
using namespace ocl;
using namespace std;

Buffer::Buffer(cl_mem mem, cl_command_queue* queue) {
	this->mem = mem;
	this->queue = queue;
}

Buffer::~Buffer() {
	clReleaseMemObject(mem);
}

cl_mem& Buffer::getMem() {
	return mem;
}

void Buffer::read(void* hostMem, const size_t size, const size_t offset, const cl_bool blocking) {
	cl_int err = 0;
	err = clEnqueueReadBuffer(*queue, mem, blocking, offset, size, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error reading Buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

void Buffer::write(const void* hostMem, const size_t size, const size_t offset, const cl_bool blocking) {
	cl_int err = 0;
	err = clEnqueueWriteBuffer(*queue, mem, blocking, offset, size, hostMem, 0, NULL, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error writing Buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

void Buffer::copy(Buffer& dst, const size_t size, const size_t srcOffset, const size_t dstOffset) {
	cl_int err = 0;
	err = clEnqueueCopyBuffer(*queue, mem, dst.mem, srcOffset, dstOffset, size, 0, NULL, NULL);
	if(err != CL_SUCCESS) {
		cout << "Error copying Buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

void* Buffer::map(const cl_map_flags flags, const size_t size, const size_t offset, const cl_bool blocking) {
	cl_int err = 0;
	void* ret = clEnqueueMapBuffer(*queue, mem, blocking, flags, offset, size, 0, NULL, NULL, &err);
	if(err != CL_SUCCESS) {
		cout << "Error mapping Buffer: " << errorMessage(err) << endl;
		exit(err);
	}
	return ret;
}

void Buffer::unmap(void* mappedPtr) {
	cl_int err = 0;
	err = clEnqueueUnmapMemObject(*queue, mem, mappedPtr, 0, NULL, NULL);
	if(err != CL_SUCCESS) {
		cout << "Error unmapping Memory Object: " << errorMessage(err) << endl;
		exit(err);
	}
}

void Buffer::copyToImage2D(Image2D& dst, const size_t size[2], const size_t srcOffset, const size_t dstOffset[2]) {
	cl_int err = 0;
	err = clEnqueueCopyBufferToImage(*queue, mem, dst.getMem(), srcOffset, dstOffset, size, 0, NULL, NULL);
	if(err != CL_SUCCESS) {
		cout << "Error copying Buffer to Image2D: " << errorMessage(err) << endl;
		exit(err);
	}
}

void Buffer::copyToImage3D(Image2D& dst, const size_t size[3], const size_t srcOffset, const size_t dstOffset[3]) {
	cl_int err = 0;
	err = clEnqueueCopyBufferToImage(*queue, mem, dst.getMem(), srcOffset, dstOffset, size, 0, NULL, NULL);
	if(err != CL_SUCCESS) {
		cout << "Error copying Buffer to Image3D: " << errorMessage(err) << endl;
		exit(err);
	}
}

void* Buffer::getMemInfo(const cl_mem_info paramName) {
	cl_int err = 0;
	size_t size;
	err = clGetMemObjectInfo (mem, paramName, 0, NULL, &size);
	if(err != CL_SUCCESS) {
		cout << "Error getting Buffer info: " << errorMessage(err) << endl;
		exit(err);
	}

	if(size > 0) {
		void* info = malloc(size);
		err = clGetMemObjectInfo (mem, paramName, size, info, &size);
		if(err != CL_SUCCESS) {
			cout << "Error getting Buffer info: " << errorMessage(err) << endl;
			exit(err);
		}
		return info;
	}
	else return NULL;
}