/**
@mainpage	OpenCL
@author		Bruno Jurkovski and Leonardo Chatain
@date		08/17/2010
**/

#ifndef __OCLPP_OPENCL_H
#define __OCLPP_OPENCL_H

#include <CL/opencl.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

#include "Program.h"
#include "Buffer.h"
#include "Image2D.h"
#include "Image3D.h"
#include "misc.h"

namespace ocl {

class OpenCL {
protected:
	cl_context context;
	cl_command_queue queue;
	cl_device_id device;
	Program* program;

	cl_platform_id platform;
	cl_uint maxComputeUnits;
	size_t maxWorkGroupSize;
	cl_ulong maxMemAllocSize;
	cl_ulong globalMemSize;
	cl_ulong constMemSize;
	cl_ulong localMemSize;

public:
	/** @brief Default constructor
	**/
	OpenCL();

	// MEMORY MANAGEMENT
	/** @brief			Creates a Buffer object
		@param[size]	is the size in bytes of the buffer memory object to be allocated.
		@param[flags]	is a bit-field that is used to specify allocation and usage information such as the memory arena that should be used to allocate the buffer object and how it will be used.
		@param[hostMem]	is a pointer to the buffer data that may already be allocated by the application. The size of the buffer that host_ptr points to must be >= size bytes. Passing in a pointer to an already allocated buffer on the host and using it as a buffer object allows applications to share data efficiently with kernels and the host.
		@return			A Buffer object.
	**/
	Buffer* createBuffer(const size_t size, const cl_mem_flags flags = CL_MEM_READ_WRITE, void* hostMem = NULL);
	Image2D* createImage2D(const size_t width, const size_t height, const size_t rowPitch, const cl_mem_flags flags, const cl_image_format* format, void* hostMem = NULL);
	Image3D* createImage3D(const size_t width, const size_t height, const size_t depth, const size_t rowPitch, const size_t slicePitch, const cl_mem_flags flags, const cl_image_format* format, void* hostMem = NULL);
	
	std::vector<cl_image_format>* getSupportedImageFormats(const cl_mem_flags flags, const cl_mem_object_type imageType);

	// KERNEL/PROGRAM MANAGEMENT
	Program* createProgram(const std::string &k);
	Program* createProgram(const std::vector<std::string> &kernels);
	Program* getProgram() const { return program; }

	void finish();

	inline cl_context getContext() const { return context; }
	inline cl_command_queue getQueue() const { return queue; }
	inline const cl_device_id& getDevice() const { return device; }

	inline cl_uint getMaxComputeUnits() const { return maxComputeUnits; }
	inline size_t getMaxWorkGroupSize() const { return maxWorkGroupSize; }
	inline cl_ulong getGlobalMemSize() const { return globalMemSize; }
	inline cl_ulong getConstMemSize() const { return constMemSize; }
	inline cl_ulong getLocalMemSize() const { return localMemSize; }
	inline cl_ulong getMaxMemAllocSize() const { return maxMemAllocSize; }

	void getDeviceInfo(const cl_device_info paramName, const size_t paramValueSize, void* paramValue, size_t* retSize);
	void getContextInfo(const cl_context_info paramName, const size_t paramValueSize, void* paramValue, size_t* retSize);
	void getCommandQueueInfo(const cl_command_queue_info paramName, const size_t paramValueSize, void* paramValue, size_t* retSize);
};

}
#endif