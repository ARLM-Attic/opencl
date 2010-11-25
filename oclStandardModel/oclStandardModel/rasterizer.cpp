#include "rasterizer.h"

using namespace std;

float* Rasterizer::loadDataset(const char* path, int& num_simplices) {
	ifstream df;
	df.open(path);
	int sx = 0, sy = 0;
	df >> sx >> sy;

	float** heights = new float*[sx];
	for (int i = 0; i < sx; i++) {
		heights[i] = new float[sy];
		for (int j = 0; j < sy; j++)
			heights[i][j] = 0;
	}
	for (int row = 0; row < sx; row++) {
		for (int col = 0; col < sy; col++) {
			df >> heights[row][col];
		}
	}
	df.close();

	num_simplices = sx*sy*2 ;
	const int size = num_simplices  * (K+1) * (N+1); // x1, x2, x3, 1
	float* simplices = new float[size];

	for (int i=0; i<size; i++)
		simplices[i] = 0;

	const int ns = num_simplices * (K+1);
	int idx = 0;
	for (int y = 0; y < sy - 1; y++) {
		for (int x = 0; x < sx - 1; x++) {
			// Even - 1st triangle
			simplices[idx] = x; // 1st coord, 1st point
			simplices[idx + ns] = y; // 2nd coord, 1st point
			simplices[idx + 2*ns] = heights[x][y]; // 3rd coord, 1st point
			simplices[idx + 3*ns] = 1; //homogenous coord, 1st point
			idx++;

			simplices[idx] = x+1; // 1st coord, 2nd point
			simplices[idx + ns] = y; // 2nd coord, 2nd point
			simplices[idx + 2*ns] = heights[x+1][y]; // 3rd coord, 2nd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 2nd point
			idx++;

			simplices[idx] = x+1; // 1st coord, 3rd point
			simplices[idx + ns] = y+1; // 2nd coord, 3rd point
			simplices[idx + 2*ns] = heights[x+1][y+1]; // 3rd coord, 3rd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 3rd point
			idx++;

			// Odd - 2nd triangle
			simplices[idx] = x; // 1st coord, 1st point
			simplices[idx + ns] = y; // 2nd coord, 1st point
			simplices[idx + 2*ns] = heights[x][y]; // 3rd coord, 1st point
			simplices[idx + 3*ns] = 1; //homogenous coord, 1st point
			idx++;

			simplices[idx] = x+1; // 1st coord, 2nd point
			simplices[idx + ns] = y+1; // 2nd coord, 2nd point
			simplices[idx + 2*ns] = heights[x+1][y+1]; // 3rd coord, 2nd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 2nd point
			idx++;

			simplices[idx] = x; // 1st coord, 3rd point
			simplices[idx + ns] = y+1; // 2nd coord, 3rd point
			simplices[idx + 2*ns] = heights[x][y+1]; // 3rd coord, 3rd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 3rd point
			idx++;
		}
	}

	for (int i = 0; i < sx; i++)
		delete[] heights[i];
	delete[] heights;

	return simplices;
}

void Rasterizer::clearVolume() {
	for(int z=0; z<GRID_SIZE_Z; z++)
		for(int y=0; y<GRID_SIZE_Y; y++)
			for(int x=0; x<GRID_SIZE_X; x++)
				volume[x][y][z] = false;
}

void Rasterizer::fillVolume() {
	for(int idx = 0; idx < SIMPLICES; idx++) {
		const int c_base = (N+1)*C_PER_SIMPLEX*idx;

		int c_counter = C_PER_SIMPLEX*(N+1);

		SMatrix points;
		// Load matrix into registers
		for (int i = 0; i < (K+1)*(N+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			points[ln][cl] = simplices[idx*(K+1) + ln*SIMPLICES*(K+1) + cl];
		}

		float minCoord[] = {9999, 9999, 9999};
		float maxCoord[] = {-9999, -9999, -9999};
		for(int coord=0; coord<N; coord++)
		{
			for(int p=0; p<N; p++)
			{
				minCoord[coord] = min(minCoord[coord], points[coord][p]);
				maxCoord[coord] = max(maxCoord[coord], points[coord][p]);
			}
			// get the floor of the min value and the ceil of the max
			minCoord[coord] = (int) minCoord[coord];
			maxCoord[coord] = (int) (maxCoord[coord] + 1);
		}

		for(int vX=(int)minCoord[0]; vX<=(int)maxCoord[0]; vX++)
			for(int vY=(int)minCoord[1]; vY<=(int)maxCoord[1]; vY++)
				for(int vZ=(int)minCoord[2]; vZ<=(int)maxCoord[2]; vZ++)
				{
					float discreteP[] = {vX, vY, vZ};

					bool raster = true;
					for(int i=0; i < c_counter/(N+1); i++)
					{
						float soma = 0;
						for(int coord=0; coord<N; coord++) {
							soma += discreteP[coord] * c_check[c_base + i*(N+1) + coord];
						}

						//if(vX==0 && vY==0 && vZ==0) cout << "fv: " << -c_check[c_base + i*(N+1) + N] << endl;

						if(!(soma <= -c_check[c_base + i*(N+1) + N])) {
							raster = false;
							break;
						}
					}
					//if(vX==0 && vY==0 && vZ==0) cout << "====" << endl;

					if(raster && vX<GRID_SIZE_X && vY<GRID_SIZE_Y && vZ<GRID_SIZE_Z && vX>=0 && vY>=0 && vZ>=0) {
						//if(vX==0 && vY==0 && vZ==0) cout << "abc " << endl;
						volume[vX][vY][vZ] = true;
					}
				}
	}
}

void Rasterizer::writeVolume(const char* filename) {
	FILE* volOut = NULL;
	if(filename) volOut = fopen(filename, "w");

	cout << "Writing volume to file... ";
	if(volOut) {
		for(int y=0; y<GRID_SIZE_Y; y++) {
			for(int z=0; z<GRID_SIZE_Z; z++) {
				for(int x=0; x<GRID_SIZE_X; x++)
					fprintf(volOut, "%d ", volume[x][y][z]);
				fprintf(volOut, "\n");
			}
			fprintf(volOut, "\n");
		}
		fclose(volOut);
		cout << "done." << endl;
	}
	else cout << "error." << endl;
}

void Rasterizer::readHeightMap(const char* filename) {
	simplices = loadDataset(filename, SIMPLICES);
	c_size = (N+1)*CONSTRAINTS;
	s_size = (K+1)*SIMPLICES;
}

void Rasterizer::readTriangles(const char* filename) {
	FILE* splxFile;
	if(filename) splxFile = fopen(filename, "r");

	cout << "Using " << filename << " simplices..." << endl;
	fscanf(splxFile, "%d", &SIMPLICES);
	c_size = (N+1)*CONSTRAINTS;
	s_size = (N+1)*(K+1)*SIMPLICES;
	simplices = new float[s_size];
	//Read the file
	//Simplex s, point p, coordinate dim
	for (int s=0; s<SIMPLICES; s++) {
		for(int dim=0; dim<N; dim++) {			
			for(int p=0; p<K+1; p++) {
				fscanf(splxFile, "%f ", &simplices[dim*(SIMPLICES)*(K+1) + s*(K+1) + p]);
				//homogenous coordinates
				simplices[N*(SIMPLICES)*(K+1) + s*(K+1) + p] = 1;
			}
		}
	}

	fclose(splxFile);
}

void Rasterizer::readInput(const char* filename, bool isHeightMap) {
	if(isHeightMap)
		readHeightMap(filename);
	else
		readTriangles(filename);
}