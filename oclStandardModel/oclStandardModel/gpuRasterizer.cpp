#include "gpuRasterizer.h"

using namespace ocl;
using namespace std;

GPU_Rasterizer::GPU_Rasterizer() : Rasterizer() {
	nck_d = cl.createBuffer(nck_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nckv);
}

void GPU_Rasterizer::initializeConstraints() {
	Rasterizer::initializeConstraints();
	constraints_d = cl.createBuffer(c_size*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, constraints);
}

void GPU_Rasterizer::readHeightMap(const char* filename) {
	Rasterizer::readHeightMap(filename);
	simplices_d = cl.createBuffer(s_size*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices);
}

void GPU_Rasterizer::readTriangles(const char* filename) {
	Rasterizer::readTriangles(filename);
	simplices_d = cl.createBuffer(s_size*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices);
}

void GPU_Rasterizer::readInput(const char* filename, bool isHeightMap) {
	Rasterizer::readInput(filename, isHeightMap);
	simplices_d = cl.createBuffer(s_size*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices);
}

void GPU_Rasterizer::clearVolume() {
	Rasterizer::clearVolume();
	volume_d = cl.createBuffer(VOLUME_SIZE*sizeof(bool), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, volume);
}

void GPU_Rasterizer::buildProgram() {
	vector<string> files;
	files.push_back("stSimplex.h");
	files.push_back("stSimplex.cl");
	Program *program = cl.createProgram(files);
	program->build(true);

	stSimplexLauncher = program->createLauncher("stSimplex");

	stSimplexLauncher.arg(simplices_d->getMem()).arg(constraints_d->getMem()).arg(nck_d->getMem()).arg(nckRows).arg(numSimplices);
	stSimplexLauncher.arg(volume_d->getMem()).arg(GRID_SIZE_X).arg(GRID_SIZE_Y).arg(GRID_SIZE_Z);
}

void GPU_Rasterizer::setWorksize(const size_t local, const size_t global) {
	stSimplexLauncher.global(global).local(local);
}

void GPU_Rasterizer::stSimplex() {
	stSimplexLauncher.run();
	cl.finish();
}

void GPU_Rasterizer::readResults() {
	constraints_d->read(constraints, c_size*sizeof(float));
	cl.finish();
}
