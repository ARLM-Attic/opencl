#include "offLoader.h"

using namespace std;

void TrianglesMesh::load(const char* filename) {
	fstream fileInput(filename);
	char OFF[4];
	fileInput.getline(OFF, 4);

	int vertexCount, faceCount, edgeCount;
	fileInput >> vertexCount;
	fileInput >> faceCount;
	fileInput >> edgeCount;

	PointXYZ* vertices = new PointXYZ[vertexCount];
	triangles = new Triangle[faceCount];

	for(int i=0; i<vertexCount; i++) {
		fileInput >> vertices[i].x;
		fileInput >> vertices[i].y;
		fileInput >> vertices[i].z;
	}

	int numVertices;
	int vertexIndex;
	for(int i=0; i<faceCount; i++) {
		fileInput >> numVertices;
		assert(numVertices==3);

		fileInput >> vertexIndex;
		triangles[i].p1 = vertices[vertexIndex];
		fileInput >> vertexIndex;
		triangles[i].p2 = vertices[vertexIndex];
		fileInput >> vertexIndex;
		triangles[i].p3 = vertices[vertexIndex];
	}

	numTriangles = faceCount;
	delete[] vertices;
	fileInput.close();
}