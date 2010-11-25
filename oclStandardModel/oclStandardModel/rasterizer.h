#ifndef _RASTERIZER_H_
#define _RASTERIZER_H_

#define GRID_SIZE_X 180
#define GRID_SIZE_Y 180
#define GRID_SIZE_Z 180
#define VOLUME_SIZE ((GRID_SIZE_X) * (GRID_SIZE_Y) * (GRID_SIZE_Z))

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <ctime>

#include <oclUtils.h>

#include "nchoosek.h"
#include "stSimplex.h"

class Rasterizer {
	protected:
		int    numSimplices;
		int    s_size;
		float* simplices;
		int    c_size;
		float* c_check;
		bool   volume[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];

		float* loadDataset(const char* path, int& num_simplices);
	public:
		Rasterizer() { }
		~Rasterizer() { }

		void clearVolume();
		void fillVolume();
		void writeVolume(const char* filename);

		void readHeightMap(const char* filename);
		void readTriangles(const char* filename);
		void readInput(const char* filename, bool isHeightMap);
};

#endif