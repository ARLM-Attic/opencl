#include "cpuRasterizer.h"

#define PROJ(ln,col) (nckv[(ln)*(N_DIMENSIONS+2)+(col)])
#define SIMP(ln,col) (simplices[s_base+(K+1)*(ln)+(col)])

using namespace std;

bool CPU_Rasterizer::isZero(float value, float tolerance) {
	return (fabs(value) < fabs(tolerance));
}

float CPU_Rasterizer::determinant(const SMatrix matrix, int size) {
	if (size == 1) {
		return matrix[0][0];
	}
	else if (size == 2) {
		float x = matrix[0][0] * matrix[1][1];
		float y = matrix[1][0] * matrix[0][1];
		return x - y;
	}
	// Sarrus
	else if (size == 3) {
		return matrix[0][0]*matrix[1][1]*matrix[2][2] + matrix[0][1]*matrix[1][2]*matrix[2][0] + matrix[1][0]*matrix[2][1]*matrix[0][2]
		- (matrix[0][2]*matrix[1][1]*matrix[2][0] + matrix[1][0]*matrix[0][1]*matrix[2][2] + matrix[0][0]*matrix[1][2]*matrix[2][1]);
	}
}

void CPU_Rasterizer::copyProjectedMatrix(SMatrix src,  SMatrix dst, const int* base) {
	for (int col = 0; col < K+1; col++) {
		for (int ln = 0; ln < N_DIMENSIONS+1; ln++) {
			dst[ln][col] = base[ln]*src[ln][col];
		}
	}
}

void CPU_Rasterizer::columnEchelonForm(SMatrix matrix, int* rank, int* independentCols) {
	*rank = 0;
	const int rows = N_DIMENSIONS+1;
	const int cols = K+1;
	for (int i=0; i!=cols; ++i) {
		independentCols[i] = i;
	}
	int c = 0;
	int r = 0;
	while ((c != cols) && (r != rows)) {
		// Find value and index of largest element in the remainder of row r.
		int k = c;
		float max = fabs(matrix[r][c]);
		for (int j=(c+1); j!=cols; ++j) {
			const float curr = fabs(matrix[r][j]);
			if (max < curr) {
				k = j;
				max = curr;
			}
		}
		if (isZero(max, TOLERANCE)) {
			// The row is negligible, zero it out.
			for (int j=c; j!=cols; ++j) {
				matrix[r][j] = 0;
			}
			++r;
		}
		else {
			float pivot;
			// Update rank.
			++(*rank);
			// Swap c-th and k-th columns.
			for (int j=0; j!=rows; ++j) {
				const float temp = matrix[j][c];
				matrix[j][c] = matrix[j][k];
				matrix[j][k] = temp;
			}
			// Swap independentCols[c] and independentCols[k]
			const int temp = independentCols[c];
			independentCols[c] = independentCols[k];
			independentCols[k] = temp;
			// Divide the pivot column by the pivot element.
			pivot = 1.0f / matrix[r][c];
			for (int j=r; j!=rows; ++j) {
				matrix[j][c] *= pivot;
			}
			// Subtract multiples of the pivot column from all the other columns.
			for (int j=0; j!=c; ++j) {
				pivot = matrix[r][j];

				for (int l=r; l!=rows; ++l) {
					matrix[l][j] -= (pivot * matrix[l][c]);
				}
			}

			for (int j=c+1; j!=cols; ++j) {
				pivot = matrix[r][j];
				for (int l=r; l!=rows; ++l) {
					matrix[l][j] -= (pivot * matrix[l][c]);
				}
			}
			++c;
			++r;
		}
	}
}

void CPU_Rasterizer::getHyperplane(SMatrix points, float* const c, const int* base, const int rank, const int* columns) {
	// Guarantees the square matrix (it's not square because of the homogeneous coordinates)
	// The matrix must actually have rank columns and rank+1 rows
	if (rank != base[N_DIMENSIONS+1]) {
		cout << "rank != base[N_DIMENSIONS+1], exiting" << endl;
		cout << "rank: " << rank << " base[N_DIMENSIONS+1]: " << base[N_DIMENSIONS+1] << endl;
		cout << "base: " << endl;
		for (int i = 0; i < N_DIMENSIONS+2; i++)
			cout << base[i] << endl;
		cout << endl << "matrix:" << endl;
		//printMatrix(points);
		exit(-1);
	}

	int dimensions[N_DIMENSIONS+1];
	int index = 0;
	int it = 0;
	// Fills the dimensions array
	for(int i=0; i<N_DIMENSIONS+1; i++)
		c[i] = 0;

	while (index < rank) {
		if (base[it] != 0) {
			dimensions[index] = it;
			index++;
		}
		it++;
	}
	dimensions[rank] = N_DIMENSIONS;		// homogeneous coordinates

	SMatrix m;

	// Initializes the first matrix (first row out) -> first coefficient
	int nCols;
	for (int ln = 0; ln < rank; ln++) {
		nCols = 0;
		int col = 0;
		//for (int col = 0; col < rank; col++) {
		while(nCols<rank) {
			if(columns[col])
				m[ln][nCols++] = points[dimensions[ln+1]][col];
			col++;
		}
			//m[ln][col] = points[dimensions[ln+1]][col];
		//}
	}
	int sig = (1 + rank+1)&1 ? -1 : 1;
	c[dimensions[0]] = sig * determinant(m, rank);
	//->certo: c[dimensions[0]] = -determinant(m, rank);
	//c[dimensions[0]] = -1*determinant(m, nCols);

	// updates the matrix -> more rank-1 coefficients
	for (int row = 0; row < rank-1; row++) {
		nCols = 0;
		int col=0;
		//for (int col = 0; col < rank; col++) {
		while(nCols<rank) {
			if(columns[col])
				m[row][nCols++] = points[dimensions[row]][col];
			col++;
			//m[row][col] = points[dimensions[row]][col];
		}
		int multiplier = ((row+1+1) + nCols+1)&1 ? -1 : 1;
		c[dimensions[row+1]] = determinant(m, nCols)*multiplier;
		//int multiplier = ((row)%2)?(-1):(1);
		////c[dimensions[row+1]] = determinant(m, rank)*multiplier;
		//->certo: c[dimensions[row+1]] = determinant(m, nCols)*multiplier;
	}

	nCols = 0;
	int col=0;
	//for (int col = 0; col < rank; col++)
	while(nCols<rank) {
		if(columns[col])
			m[rank-1][nCols++] = points[dimensions[rank-1]][col];
		col++;
	}
		//m[rank-1][col] = points[dimensions[rank-1]][col];
	// last one is b
	////c[N_DIMENSIONS] = determinant(m, rank);
	//->certo: c[N_DIMENSIONS] = determinant(m, nCols);
	sig = (rank+1 + nCols+1)&1 ? -1 : 1;
	c[N_DIMENSIONS] = sig*determinant(m, nCols);
}

// Calculates the correct constraint for the given half-space
void CPU_Rasterizer::halfSpaceConstraints(float* const coefficients) {
	float b = 0;
	for (int i = 0; i < N_DIMENSIONS; i++) {
		b += fabs(coefficients[i]);
	}
	b = b/2;
	//coefficients[N_DIMENSIONS] += b;
	coefficients[N_DIMENSIONS] -= b;
}


void CPU_Rasterizer::makeStandardOriented(float* const coefficients) {
	bool hasStdOrientation = true;
	for(int i=0; i<=N_DIMENSIONS; i++)
		if(coefficients[i] < -TOLERANCE) {
			hasStdOrientation = false;
			break;
		}

	if(!hasStdOrientation)
		for(int i=0; i<=N_DIMENSIONS; i++)
			coefficients[i] = -coefficients[i];
}


void CPU_Rasterizer::triangleFaceOrientation(const SMatrix points, const int rank,
							const int* const columns, float* const coefficients) {
	for(int p=0; p<rank; p++)
		if(columns[p]==0)
		{
			float dot = 0;
			for(int coord=0; coord<=N_DIMENSIONS; coord++)
				dot += coefficients[coord]*points[coord][p];

			if(dot > 0) {
				for(int i=0; i<N_DIMENSIONS+1; i++)
					coefficients[i] = -coefficients[i];
				break;
			}
		}
}

//////////////////////////////////////////////////////////////////////////////

void CPU_Rasterizer::echelonTest(int* ranks, int* indCols) {
	//for (int i = 0; i < SIMPLICES; i++) { //temp
	for (int i = 0; i < numSimplices; i++) {
		SMatrix e;
		const int s_base = (N_DIMENSIONS+1)*(K+1)*i;
		//copy matrix
		for (int j = 0; j < (N_DIMENSIONS+1)*(K+1); j++) {
			const int ln = j/(K+1);
			const int cl = j%(K+1);
			e[ln][cl]=simplices[s_base+j];
		}
		// echelon form
		int ic[K+1];
		columnEchelonForm(e, &ranks[i], ic);
		const int ic_base = i*(K+1);
		for (int i = 0; i< (K+1); i++)
			indCols[ic_base+i] = ic[i];
		//copy back
		for (int j = 0; j < (N_DIMENSIONS+1)*(K+1); j++) {
			const int ln = j/(K+1);
			const int cl = j%(K+1);
			simplices[s_base+j]=e[ln][cl];
		}
	}
}

void CPU_Rasterizer::stSimplex()
{
	//for (int idx = 0; idx < SIMPLICES; idx++) { //temp
	for (int idx = 0; idx < numSimplices; idx++) {
		//cout << "simplex: " << idx << endl;
		const int s_base = (N_DIMENSIONS+1)*(K+1)*idx;
		const int ic_base = (K+1)*idx;
		const int c_base = (N_DIMENSIONS+1)*C_PER_SIMPLEX*idx;

		int c_counter = 0;

		SMatrix echelon, points;
		
		// Load matrix into registers
		for (int i = 0; i < (K+1)*(N_DIMENSIONS+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			points[ln][cl] = simplices[idx*(K+1) + ln*numSimplices*(K+1) + cl];
			//points[ln][cl] = simplices[s_base+i];
		}

		//cout << "c_base " << c_base << endl;
		//cout << "ic_base: " << ic_base << endl;
		//cout << "s_base: " << s_base << endl;

		//cout << "Points: " << endl;
		//printMatrix(points);

		// Iterates through all possible projections
		for (int d = 0; d < nckRows; d++) {
			const int dim = PROJ(d,N_DIMENSIONS+1);
			const int* proj_base = &PROJ(d,0);
			//Copies the projected matrix to echelon
			copyProjectedMatrix(points, echelon, proj_base);

			// Compute matrix representation of the flat induced by the vertices
			// projected onto current d-dimensional space (i.e., the reduced
			// column echelon form of vertices_proj) and its dimensionality
			// (i.e., the rank of flat's matrix).
			int rank, ic[K+1];
			columnEchelonForm(echelon, &rank, ic);

			// Compute constraints from induced flat if it is an hyperplane in
			// current d-dimensional space.
			if (rank == dim) {
				constraint c;
				int columns[K+1];
				for (int i = 0; i < K+1; i++) columns[i] = 1;
				//printf("rank==dim\n");
				getHyperplane(echelon, c, proj_base, rank, columns);
				makeStandardOriented(c);
				halfSpaceConstraints(c);
				for (int i = 0; i < (N_DIMENSIONS+1); i++) {
					constraints[c_base+c_counter] = c[i];
					c_counter++;
				}

			}
			else if (rank == dim+1) {
				if (dim > 1) {
					// Dimension is higher than 1,
					// create constraints from the extrusion of half-spaces bounded
					// by the facets of the convex polytope.
					// *NOTE*: here we assume that there are only triangles (2-simplex)
					// So, now we must create 3 new constraints, relative to facets 1-2, 1-3, 2-3
					int columns[K+1];
					for (int i = 1; i < K+1; i++) columns[i] = 1;
					columns[0] = 0;
					constraint c;

					getHyperplane(points, c, proj_base, rank-1, columns);
					triangleFaceOrientation(points, rank, columns, c);
					halfSpaceConstraints(c);
					for (int i = 0; i < (N_DIMENSIONS+1); i++) {
						constraints[c_base+c_counter] = c[i];
						c_counter++;
					}

					for (int i = 1; i < K+1; i++) {
						columns[i-1] = 1; columns[i] = 0;
						getHyperplane(points, c, proj_base, rank-1, columns);
						triangleFaceOrientation(points, rank, columns, c);
						halfSpaceConstraints(c);
						for (int i = 0; i < (N_DIMENSIONS+1); i++) {
							constraints[c_base+c_counter] = c[i];
							c_counter++;
						}
					}

				} else {
					float minC;
					float maxC;
					int coord;

					for(int i=0; i<N_DIMENSIONS; i++)
						if(proj_base[i])
							coord = i;

					minC = 999999;//numeric_limits<float>::max();
					maxC = -999999;//numeric_limits<float>::min();
					for(int p=0; p<N_DIMENSIONS; p++)
					{
						minC = min(minC, points[coord][p]);
						maxC = max(maxC, points[coord][p]);
					}

					constraint consMin;
					constraint consMax;

					for(int i=0; i<N_DIMENSIONS; i++)
					{
						if(i==coord)
							consMin[i] = -1;
						else
							consMin[i] = 0;
					}
					consMin[N_DIMENSIONS] = minC;

					for(int i=0; i<N_DIMENSIONS; i++)
					{
						if(i==coord)
							consMax[i] = 1;
						else
							consMax[i] = 0;
					}
					consMax[N_DIMENSIONS] = -maxC;

					halfSpaceConstraints(consMin);
					halfSpaceConstraints(consMax);

					for (int i = 0; i < (N_DIMENSIONS+1); i++) {
						constraints[c_base+c_counter] = consMin[i];
						c_counter++;
					}

					for (int i = 0; i < (N_DIMENSIONS+1); i++) {
						constraints[c_base+c_counter] = consMax[i];
						c_counter++;
					}
				}
			}
		}
	}
}