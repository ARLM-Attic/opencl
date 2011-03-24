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

		//
		// Loads an OFF file just with triangles. DON'T accept other polygons.
		//
		void load(const char* filename);
};

//
//	Converts an OFF file to a Triangles mesh file in the following format:
//
//
//		number_of_triangles
//
//		x1(1) x2(1) x3(1)
//		y1(1) y2(1) y3(1)
//		z1(1) z2(1) z3(1)
//
//			  ...
//
//		x1(number_of_triangles) x2(number_of_triangles) x3(number_of_triangles)
//		y1(number_of_triangles) y2(number_of_triangles) y3(number_of_triangles)
//		z1(number_of_triangles) z2(number_of_triangles) z3(number_of_triangles)
//
//
//	[Where (x_i(T), y_i(T), z_i(T)) is the cartesian coordinate of the i-th
//	 point of triangle T]
//
//	The original mesh is also scaled by "scale" factor.
//
void offToTriangles(const char* offFilename, const char* outputFilename, float scale);

#endif