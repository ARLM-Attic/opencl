#include "offLoader.h"

using namespace std;

void TrianglesMesh::load(const char* filename) {
	fstream fileInput(filename);
	char OFF[4];
	fileInput.getline(OFF, 4);
	assert(!strcmp(OFF, "OFF"));

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

		for(int j=0; j<numVertices; j++) {
			fileInput >> vertexIndex;
			triangles[i].point[j] = vertices[vertexIndex];
		}
	}

	numTriangles = faceCount;
	delete[] vertices;
	fileInput.close();
}

void offToTriangles(const char* offFilename, const char* outputFilename, float scale) {
	TrianglesMesh mesh;
	mesh.load(offFilename);

	float minX=99999, minY=99999, minZ=99999;
	float maxX=-99999, maxY=-99999, maxZ=-99999;
	for(int i=0; i<mesh.numTriangles; i++) {
		for(int j=0; j<3; j++) {
			if(mesh.triangles[i].point[j].x < minX) minX = mesh.triangles[i].point[j].x;
			if(mesh.triangles[i].point[j].x > maxX) maxX = mesh.triangles[i].point[j].x;
			if(mesh.triangles[i].point[j].y < minY) minY = mesh.triangles[i].point[j].y;
			if(mesh.triangles[i].point[j].y > maxY) maxY = mesh.triangles[i].point[j].y;
			if(mesh.triangles[i].point[j].z < minZ) minZ = mesh.triangles[i].point[j].z;
			if(mesh.triangles[i].point[j].z > maxZ) maxZ = mesh.triangles[i].point[j].z;
		}
	}

	int dists[3];
	dists[0] = maxX-minX;
	dists[1] = maxY-minY;
	dists[2] = maxZ-minZ;
	int maxDist = -1, maxDistIdx = -1;
	for(int i=0; i<3; i++)
		if(dists[i]>maxDist) {
			maxDistIdx = i;
			maxDist = dists[i];
		}

		if(scale<0) scale = 1;
		else scale = scale/(float)maxDist;

		FILE* fTriangles = fopen(outputFilename, "w");
		fprintf(fTriangles, "%d\n\n", mesh.numTriangles);
		for(int i=0; i<mesh.numTriangles; i++) {
			fprintf(fTriangles, "%f %f %f\n", scale*(mesh.triangles[i].point[0].x-minX), scale*(mesh.triangles[i].point[1].x-minX), scale*(mesh.triangles[i].point[2].x-minX));
			fprintf(fTriangles, "%f %f %f\n", scale*(mesh.triangles[i].point[0].y-minY), scale*(mesh.triangles[i].point[1].y-minY), scale*(mesh.triangles[i].point[2].y-minY));
			fprintf(fTriangles, "%f %f %f\n", scale*(mesh.triangles[i].point[0].z-minZ), scale*(mesh.triangles[i].point[1].z-minZ), scale*(mesh.triangles[i].point[2].z-minZ));
			fprintf(fTriangles, "\n");
		}
		fclose(fTriangles);
}