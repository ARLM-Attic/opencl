#ifndef __OCLPP_OPENCL_H
#define __OCLPP_OPENCL_H

#include <CL/opencl.h>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>

#include "Program.h"
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
	OpenCL();

	// MEMORY MANAGEMENT
	cl_mem createBuffer(const size_t size, const cl_mem_flags flags = CL_MEM_READ_WRITE, void* hostMem = NULL);
	void readBuffer(cl_mem deviceMem, void* hostMem, const size_t size, const size_t offset = 0);
	void writeBuffer(cl_mem deviceMem, void* hostMem, const size_t size, const size_t offset = 0);

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

};

}
#endif