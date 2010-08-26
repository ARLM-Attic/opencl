#include "OpenCL.h"

using namespace std;
using namespace ocl;

OpenCL::OpenCL() {
	cl_int error = 0;
	error = clGetPlatformIDs(1, &platform, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error getting platform id: " << errorMessage(error) << endl;
		exit(error);
	}
	error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (error != CL_SUCCESS) {
		cout << "Error getting device ids: " << errorMessage(error) << endl;
		exit(error);
	}
	context = clCreateContext(0, 1, &device, NULL, NULL, &error);
	//context = clCreateContextFromType(platform, CL_DEVICE_TYPE_GPU, NULL, NULL, &error);
	if (error != CL_SUCCESS) {
		cout << "Error creating context: " << errorMessage(error) << endl;
		exit(error);
	}

	/*size_t devices_size;
	error = clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &devices_size);
	device = (cl_device_id*) malloc (devices_size);
	error |= clGetContextInfo(context, CL_CONTEXT_DEVICES, devices_size, device, 0);
	if (error != CL_SUCCESS) {
		cout << "Error getting context information: " << errorMessage(error) << endl;
		exit (error);
	}*/
	queue = clCreateCommandQueue(context, device, 0, &error);
	if (error != CL_SUCCESS) {
		cout << "Error creating command queue: " << errorMessage(error) << endl;
		exit(error);
	}

	// Getting some information about the device
	clGetDeviceInfo(device, CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &maxComputeUnits, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_MEM_ALLOC_SIZE, sizeof(cl_ulong), &maxMemAllocSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &globalMemSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_MAX_CONSTANT_BUFFER_SIZE, sizeof(cl_ulong), &constMemSize, NULL);
	clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemSize, NULL);

}

Buffer* OpenCL::createBuffer(const size_t size, const cl_mem_flags flags, void* hostMem) {
	cl_int err = 0;
	cl_mem buf = clCreateBuffer(context, flags, size, hostMem, &err);
	if (err == CL_SUCCESS) {
		Buffer* ret = new Buffer(buf, &queue);
		return ret;
	} else {
		cout << "Error creating buffer: " << errorMessage(err) << endl;
		exit(err);
	}
}

Image2D* OpenCL::createImage2D(const size_t width, const size_t height, const size_t rowPitch, const cl_mem_flags flags, const cl_image_format* format, void* hostMem) {
	cl_int err = 0;
	cl_mem im2d = clCreateImage2D(context, flags, format, width, height, rowPitch, hostMem, &err);
	if(err != CL_SUCCESS) {
		cout << "Error creating Image2D: " << errorMessage(err) << endl;
		exit(err);
	}
	Image2D* ret = new Image2D(im2d, rowPitch, &queue);
	return ret;
}

Image3D* OpenCL::createImage3D(const size_t width, const size_t height, const size_t depth, const size_t rowPitch, const size_t slicePitch, const cl_mem_flags flags, const cl_image_format* format, void* hostMem) {
	cl_int err = 0;
	cl_mem im3d = clCreateImage3D(context, flags, format, width, height, depth, rowPitch, slicePitch, hostMem, &err);
	if(err != CL_SUCCESS) {
		cout << "Error creating Image3D: " << errorMessage(err) << endl;
		exit(err);
	}
	Image3D* ret = new Image3D(im3d, rowPitch, slicePitch, &queue);
	return ret;
}

Program* OpenCL::createProgram(const vector<string> &kernels) {
	program = new Program(kernels, &context, &queue, &device);
	return program;
}

Program* OpenCL::createProgram(const string &k) {
	vector<string> v(1);
	v[0] = k;
	return createProgram(v);
}

void OpenCL::finish() {
	clFinish(queue);
}

void* OpenCL::getDeviceInfo(const cl_device_info paramName) {
	cl_int err=0;
	size_t size;
	err = clGetDeviceInfo (device, paramName, 0, NULL, &size);
	if(err != CL_SUCCESS) {
		cout << "Error getting device info: " << errorMessage(err) << endl;
		exit(err);
	}
	
	if(size > 0) {
		void* info = malloc(size);
		err = clGetDeviceInfo(device, paramName, size, info, &size);
		if(err != CL_SUCCESS) {
			cout << "Error getting device info: " << errorMessage(err) << endl;
			exit(err);
		}
		return info;
	}
	else return NULL;
}

void* OpenCL::getContextInfo(const cl_context_info paramName) {
	cl_int err = 0;
	size_t size;
	err = clGetContextInfo(context, paramName, 0, NULL, &size);
	if(err != CL_SUCCESS) {
		cout << "Error getting context info: " << errorMessage(err) << endl;
		exit(err);
	}

	if(size > 0) {
		void* info = malloc(size);
		err = clGetContextInfo(context, paramName, size, info, &size);
		if(err != CL_SUCCESS) {
			cout << "Error getting context info: " << errorMessage(err) << endl;
			exit(err);
		}
		return info;
	}
	else return NULL;
}

void* OpenCL::getCommandQueueInfo(const cl_command_queue_info paramName) {
	cl_int err = 0;
	size_t size;
	err = clGetCommandQueueInfo(queue, paramName, 0, NULL, &size);
	if(err != CL_SUCCESS) {
		cout << "Error getting command queue info: " << errorMessage(err) << endl;
		exit(err);
	}

	if(size > 0) {
		void* info = malloc(size);
		err = clGetCommandQueueInfo(queue, paramName, size, info, &size);
		if(err != CL_SUCCESS) {
			cout << "Error getting command queue info: " << errorMessage(err) << endl;
			exit(err);
		}
		return info;
	}
	else return NULL;
}

vector<cl_image_format>* OpenCL::getSupportedImageFormats(const cl_mem_flags flags, const cl_mem_object_type imageType) {
	cl_uint numEntries;
	cl_int err = clGetSupportedImageFormats(context, flags, imageType, 0, NULL, &numEntries);
	if (err != CL_SUCCESS) {
		cout << "0 Image formats supported: " << errorMessage(err) << endl;
		exit(err);
	}

	cl_image_format* value = (cl_image_format*) malloc(numEntries * sizeof(cl_image_format));
	err = clGetSupportedImageFormats(context, flags, imageType, numEntries, value, NULL);
	if (err != CL_SUCCESS) {
		cout << "Error getting supported Image formats: " << errorMessage(err) << endl;
		exit(err);
	}

	vector<cl_image_format>* formats = new vector<cl_image_format>(numEntries);
	formats->assign(&value[0], &value[numEntries]);
	return formats;
}
