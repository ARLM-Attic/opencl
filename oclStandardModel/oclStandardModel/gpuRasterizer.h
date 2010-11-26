#ifndef _GPU_RASTERIZER_H_
#define _GPU_RASTERIZER_H_

#include "rasterizer.h"

#include "OpenCL.h"


class GPU_Rasterizer : public Rasterizer {
	protected:
		ocl::OpenCL   cl;
		ocl::Buffer*  pbo_cl;
		ocl::Program* progStRender;
		ocl::Launcher stRender;
		ocl::Launcher stFillVolume;
		ocl::Buffer*  constraints_d;
		ocl::Buffer*  simplices_d;
		ocl::Buffer*  volume_d;
		ocl::Buffer*  nck_d;
		ocl::Launcher stSimplexLauncher;

	public:
		GPU_Rasterizer();
		~GPU_Rasterizer() { }

		void clearVolume();

		void initializeConstraints();

		void readHeightMap(const char* filename);
		void readTriangles(const char* filename);
		void readInput(const char* filename, bool isHeightMap);

		void buildProgram();
		void setWorksize(const size_t local, const size_t global);
		void stSimplex();
		void readResults();
};

#endif