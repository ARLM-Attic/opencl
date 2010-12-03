#ifndef _OFF_LOADER_H_
#define _OFF_LOADER_H_

#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <cassert>

typedef struct {
	float x;
	float y;
	float z;
} PointXYZ;

typedef struct {
	PointXYZ point[3];
} Triangle;

class TrianglesMesh {
	public:
		int numTriangles;
		Triangle* triangles;

		void load(const char* filename);
};

void offToTriangles(const char* offFilename, const char* outputFilename, float scale);

#endif