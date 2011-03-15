#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

/* Rasterizer Volume Dimensions */
//*
#define GRID_SIZE_X 180
#define GRID_SIZE_Y 180
#define GRID_SIZE_Z 180
//*/
/*
#define GRID_SIZE_X 512
#define GRID_SIZE_Y 512
#define GRID_SIZE_Z 512
//*/
#define VOLUME_SIZE ((GRID_SIZE_X) * (GRID_SIZE_Y) * (GRID_SIZE_Z))

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <ctime>
#include <cmath>

#include "nchoosek.h"
#include "stSimplex.h"

class Rasterizer {
	protected:
		//static const int N_DIMENSIONS;
		//static const int K;
		int    numSimplices;
		int    s_size;
		float* simplices;
		int    c_size;
		int*   nckv;
		int    nckRows;
		int    nck_size;
		float* constraints;

		//
		// Load a dataset in the following format (heightmap):
		//
		//	width length
		//	Z(1,1) Z(2,1) Z(3,1) ... Z(width, 1)
		//	Z(1,2) Z(2,2) ...
		//	...
		//	Z(1,length) Z(2,length) ... Z(width,length)
		//
		// [where Z(x,y) is the height of (x,y) point]
		//
		float* loadDataset(const char* path, int& num_simplices);
	public:
		bool   volume[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];

		Rasterizer();
		~Rasterizer() { }

		int getNumSimplices();

		//
		//	Test each voxel in the Rasterizer volume against all
		//	the constraints computed. If it pass all the tests, mark
		//	it to be displayed.
		//
		void fillVolume();
		void clearVolume();
		void writeVolume(const char* filename);

		//
		// Load an input in the following format (mesh of triangles):
		//
		//	number_of_triangles
		//
		//	x1(1) x2(1) x3(1)
		/// y1(1) y2(1) y3(1)
		//	z1(1) z2(1) z3(1)
		//
		//	x1(2) x2(2) x3(2)
		/// y1(2) y2(2) y3(2)
		//	z1(2) z2(2) z3(2)
		//
		//		  ...
		//
		//	x1(N) x2(N) x3(N)
		/// y1(N) y2(N) y3(N)
		//	z1(N) z2(N) z3(N)
		//
		// Where the number in parentheses is the triangle ID (it doesn't
		// appear in the file, this example is just to illustrate)
		//
		void readTriangles(const char* filename);
		void readHeightMap(const char* filename);
		void readInput(const char* filename, bool isHeightMap);

		void initializeConstraints();

		void printConstraints(FILE* file=NULL);

		//
		// Compare the computed constraints with the ones contained in the 'filename' file.
		//
		void compareResults(const char* filename);
};

#endif