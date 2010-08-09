#include <iostream>
#include "OpenCL.h"
#include "Timer.h"

using namespace std;

int main()
{
	ocl::OpenCL opencl;
	vector<string> files;
	files.push_back("kernels.cl");

	ocl::Program* prog = opencl.createProgram(files);
	prog->build();

	const int SIZE = 1024;
	float* src_a_h = new float[SIZE];
	float* src_b_h = new float[SIZE];
	float* dst_h = new float[SIZE];

	for(int i=0; i<SIZE; i++) {
		src_a_h[i] = i;
		src_b_h[i] = i;
	}

	ocl::Timer timer;
	double time;

	timer.start();
	cl_mem src_a_d = opencl.createBuffer(SIZE*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, src_a_h);
	cl_mem src_b_d = opencl.createBuffer(SIZE*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, src_b_h);
	cl_mem dst_d = opencl.createBuffer(SIZE*sizeof(float), CL_MEM_WRITE_ONLY);
	opencl.finish();
	time = timer.getTime();
	cout << "Buffers Creation Time: " << time << endl;

	ocl::Launcher kernelLauncher = prog->createLauncher("add_vectors");

	const size_t global_ws = SIZE, local_ws = 512;
	kernelLauncher.global(global_ws).local(local_ws);
	kernelLauncher.arg(src_a_d).arg(src_b_d).arg(dst_d).arg(SIZE);
	timer.start();
	kernelLauncher.run();
	opencl.finish();
	time = timer.getTime();
	cout << "Kernel Run Time: " << time << endl;

	opencl.readBuffer(dst_d, dst_h, SIZE*sizeof(float));
	opencl.finish();

	for(int i=0; i<SIZE; i++)
		assert(dst_h[i] == src_a_h[i] + src_b_h[i]);

	getchar();
	return 0;
}