#define PROJ(ln, col) (proj[(ln)*(N+2)+(col)])
#define PROJ_BASE (&(proj[(d)*(N+2)]))


void columnEchelonForm(SMatrix matrix, int* const rank, int* const independentCols);

float determinant(const SMatrix matrix, const int size);
void getHyperplane(const SMatrix points,float* const c, __constant const int* const base,
				   const int rank, const int* const columns);

void copyProjectedMatrix(const SMatrix src, SMatrix dst, __constant const int* const base);

bool hasStandardOrientation(const float* const coefficients);
void halfSpaceConstraints(float* const coefficients);

void makeStandardOriented(float* const coefficients);
void triangleFaceOrientation(const SMatrix points, const int rank,
							const int* const columns, float* const coefficients);

//////////////////////////////////////////////////////////////////////////////

__kernel void stSimplex(__global const float* const simplices,
						__global float* const constraints,
						__constant const int* const proj,
						const int projRows,
						const int numSimplices,
						__global int* const volume,
						const int volumeW, const int volumeH, const int volumeD)
{
	const int idx = get_global_id(0);
	//if (idx < SIMPLICES) { //temp
	if (idx < numSimplices) {
		const int s_base = (N+1)*(K+1)*idx;
		const int ic_base = (K+1)*idx;
		const int c_base = (N+1)*C_PER_SIMPLEX*idx;

		int c_counter = 0;

		SMatrix echelon, points;
		// Load matrix into registers
		for (int i = 0; i < (K+1)*(N+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			points[ln][cl] = simplices[idx*(K+1) + ln*numSimplices*(K+1) + cl];
			//points[ln][cl] = simplices[s_base+i];
		}
		
		// Iterates through all possible projections
		for (int d = 0; d < projRows; d++) {
			const int dim = PROJ(d,N+1);
			__constant const int* proj_base = &(PROJ(d,0));
			//__constant const int* const proj_base = &proj[(d)*(N+2)];
			//Copies the projected matrix to echelon
			copyProjectedMatrix(points, echelon, PROJ_BASE);

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
				getHyperplane(echelon, c, PROJ_BASE, rank, columns);
				makeStandardOriented(c);
				halfSpaceConstraints(c);
				for (int i = 0; i < (N+1); i++) {
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
					getHyperplane(points, c, PROJ_BASE, rank-1, columns);
					triangleFaceOrientation(points, rank, columns, c);
					halfSpaceConstraints(c);
					for (int i = 0; i < (N+1); i++) {
						constraints[c_base+c_counter] = c[i];
						c_counter++;
					}
					for (int i = 1; i < K+1; i++) {
						columns[i-1] = 1; columns[i] = 0;
						getHyperplane(points, c, PROJ_BASE, rank-1, columns);
						triangleFaceOrientation(points, rank, columns, c);
						halfSpaceConstraints(c);
						for (int i = 0; i < (N+1); i++) {
							constraints[c_base+c_counter] = c[i];
							c_counter++;
						}
					}

				} else {
					float minC;
					float maxC;
					int coord;

					for(int i=0; i<N; i++)
						if(proj_base[i])
							coord = i;

					minC = 9999;
					maxC = -minC;
					for(int p=0; p<N; p++)
					{
						minC = min(minC, points[coord][p]);
						maxC = max(maxC, points[coord][p]);
					}

					constraint consMin;
					constraint consMax;

					for(int i=0; i<N; i++)
					{
						if(i==coord)
							consMin[i] = -1;
						else
							consMin[i] = 0;
					}
					consMin[N] = minC;

					for(int i=0; i<N; i++)
					{
						if(i==coord)
							consMax[i] = 1;
						else
							consMax[i] = 0;
					}
					consMax[N] = -maxC;

					halfSpaceConstraints(consMin);
					halfSpaceConstraints(consMax);

					for (int i = 0; i < (N+1); i++) {
						constraints[c_base+c_counter] = consMin[i];
						c_counter++;
					}

					for (int i = 0; i < (N+1); i++) {
						constraints[c_base+c_counter] = consMax[i];
						c_counter++;
					}
				}
			}
		}

		/*
		// Fill a volume:
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
							soma += discreteP[coord] * constraints[c_base + i*(N+1) + coord];
						}

						if(!(soma <= -constraints[c_base + i*(N+1) + N])) {
							raster = false;
							break;
						}
					}
					
					if(raster && vX<volumeW && vY<volumeH && vZ<volumeD && vX>=0 && vY>=0 && vZ>=0) {
						volume[vX*volumeH*volumeD + vY*volumeD + vZ] = 1;
					}
				}
//*/
	}
}

//////////////////////////////////////////////////////////////////////////////

bool hasStandardOrientation(const float* const coefficients) {
	for (int i = 0; i < N; i++) {
		if (coefficients[i] > 0)
			return true;
		else if (coefficients[i] < 0)
			return false;
	}
	return false;
}

// Calculates the correct constraint for the given half-space
// constraint = sum(Coefficient[i])/2
void halfSpaceConstraints(float* const coefficients) {
	float b = 0;
	for (int i = 0; i < N; i++) {
		b += fabs(coefficients[i]);
	}
	b = b/2;
	coefficients[N] -= b;
}

void makeStandardOriented(float* const coefficients) {
	bool hasStdOrientation = true;
	for(int i=0; i<=N; i++)
		if(coefficients[i] < -TOLERANCE) {
			hasStdOrientation = false;
			break;
		}

	if(!hasStdOrientation)
		for(int i=0; i<=N; i++)
			coefficients[i] = -coefficients[i];
}


void triangleFaceOrientation(const SMatrix points, const int rank,
							const int* const columns, float* const coefficients) {
	for(int p=0; p<rank; p++)
		if(columns[p]==0)
		{
			float dot = 0;
			for(int coord=0; coord<=N; coord++)
				dot += coefficients[coord]*points[coord][p];

			if(dot > 0) {
				for(int i=0; i<N+1; i++)
					coefficients[i] = -coefficients[i];
				break;
			}
		}
}

void copyProjectedMatrix(const SMatrix src,
						 SMatrix dst,
						 __constant const int* const base)
{
	for (int col = 0; col < K+1; col++) {
		for (int ln = 0; ln < N+1; ln++) {
			dst[ln][col] = base[ln]*src[ln][col];
		}
	}
}

bool isZero(const float value, const float tolerance) {
	return (fabs(value) < fabs(tolerance));
}

void columnEchelonForm(SMatrix matrix,
					   int* const rank,
					   int* const independentCols)
{
	*rank = 0;
	const int rows = N+1;
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

// MAX matrix size == 3x3
float determinant(const SMatrix matrix, const int size) {
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

void getHyperplane2(const SMatrix points,
				   float* const c,
				   __constant const int* const basis,
				   const int rank) {
	SMatrix m;

	// Initializes the first matrix (first col out) -> first coefficient
	for (int h = 0; h < rank; h++) {
		for (int col = 0; col < rank; col++) {
			if (col < rank-1)
				m[col][h] = points[h][col+1];
			else
				m[col][h] = 1;
		}
	}
	c[0] = determinant(m, rank);

	// updates the matrix -> more n-1 coefficients
	for (int col = 0; col < rank-1; col++) {
		for (int h = 0; h < rank; h++) {
			m[col][h] = points[h][col];
		}
		int multiplier = ((col+1)%2)?(-1):(1);
		c[col+1] = determinant(m, rank) * multiplier;
	}

	// last one is b
	for (int h = 0; h < rank; h++)
		m[rank-1][h] = points[h][rank-1];
	c[N] = determinant(m, rank) * -1;
}

void getHyperplane(const SMatrix points,
				   float* const c,
				   __constant const int* const base,
				   const int rank,
				   const int* const columns) {
	// Guarantees the square matrix (it's not square because of the homogeneous coordinates)
	// The matrix must actually have rank columns and rank+1 rows

	int dimensions[N+1];
	int index = 0;
	int it = 0;
	// Fills the dimensions array
	for(int i=0; i<N+1; i++)
		c[i] = 0;

	while (index < rank) {
		if (base[it] != 0) {
			dimensions[index] = it;
			index++;
		}
		it++;
	}
	dimensions[rank] = N;		// homogeneous coordinates
	
	SMatrix m;

	//SMatrix points

	// Initializes the first matrix (first row out) -> first coefficient
	int nCols = 0;
	for (int ln = 0; ln < rank; ln++) {
		nCols = 0;
		int col = 0;
		while(nCols<rank) {
			if(columns[col])
				m[ln][nCols++] = points[dimensions[ln+1]][col];
			col++;
		}
	}
	int sig = (1 + rank+1)&1 ? -1 : 1;
	c[dimensions[0]] = sig * determinant(m, rank);

	// updates the matrix -> more rank-1 coefficients
	for (int row = 0; row < rank-1; row++) {
		nCols = 0;
		int col = 0;
		while(nCols<rank) {
			if(columns[col])
				m[row][nCols++] = points[dimensions[row]][col];
			col++;
		}
		int multiplier = ((row+1+1) + nCols+1)&1 ? -1 : 1;
		c[dimensions[row+1]] = determinant(m, nCols)*multiplier;
	}

	nCols = 0;
	int col = 0;
	while(nCols<rank) {
		if(columns[col])
			m[rank-1][nCols++] = points[dimensions[rank-1]][col];
		col++;
	}
	sig = (rank+1 + nCols+1)&1 ? -1 : 1;
	c[N] = sig*determinant(m, nCols);
}


