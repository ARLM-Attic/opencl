#ifndef _GPU_RASTERIZER_H_
#define _GPU_RASTERIZER_H_

#include "rasterizer.h"

#include "OpenCL.h"
#include <oclUtils.h>

#include <vector>

class GPU_Rasterizer : public Rasterizer {
	protected:
		ocl::OpenCL   cl;
		ocl::Buffer*  pbo_cl;
		ocl::Program* progStRender;
		/* GPU Kernel Launchers */
		ocl::Launcher stRender;
		ocl::Launcher stFillVolume;
		ocl::Launcher stSimplexLauncher;
		/* Device memory pointers */
		ocl::Buffer*  constraints_d;
		ocl::Buffer*  simplices_d;
		ocl::Buffer*  volume_d;
		ocl::Buffer*  nck_d;

	public:
		GPU_Rasterizer();
		~GPU_Rasterizer() { }

		void clearVolume();
		void initializeConstraints();

		/* Overloading Rasterizer input loading methods. See rasterizer.h for more info */
		void readTriangles(const char* filename);
		void readHeightMap(const char* filename);
		void readInput(const char* filename, bool isHeightMap);

		/* Logic is in stSimplex2.cl */
		void buildProgram();
		void setWorksize(const size_t local, const size_t global);
		void stSimplex();
		void readResults();
};

#endif