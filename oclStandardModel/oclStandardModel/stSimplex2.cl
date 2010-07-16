#define PROJ(ln, col) (proj[(ln)*(N+2)+(col)])
#define PROJ_BASE (&(proj[(d)*(N+2)]))


void columnEchelonForm(SMatrix matrix, int* const rank, int* const independentCols);

float determinant(const SMatrix matrix, const int size);
void getHyperplane(const SMatrix points,float* const c, __constant const int* const base,
				   const int rank, const int* const columns);

void copyProjectedMatrix(const SMatrix src, SMatrix dst, __constant const int* const base);

bool hasStandardOrientation(const float* const coefficients);
void halfSpaceConstraints(float* const coefficients);

//////////////////////////////////////////////////////////////////////////////

__kernel void stSimplex(__global const float* const simplices,
						__global float* const constraints,
						__constant const int* const proj,
						const int projRows)
{
	const int idx = get_global_id(0);
	if (idx < SIMPLICES) {
		const int s_base = (N+1)*(K+1)*idx;
		const int ic_base = (K+1)*idx;
		const int c_base = (N+1)*C_PER_SIMPLEX*idx;

		int c_counter = 0;

		SMatrix echelon, points;
		// Load matrix into registers
		for (int i = 0; i < (K+1)*(N+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			points[ln][cl] = simplices[s_base+i];
		}
		
		// Iterates through all possible projections
		for (int d = 0; d < projRows; d++) {
			const int dim = PROJ(d,N+1);
			//__constant const int* proj_base = &(PROJ(d,0));
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
					halfSpaceConstraints(c);
					for (int i = 0; i < (N+1); i++) {
						constraints[c_base+c_counter] = c[i];
						c_counter++;
					}
					for (int i = 1; i < K+1; i++) {
						columns[i-1] = 1; columns[i] = 0;
						getHyperplane(points, c, PROJ_BASE, rank-1, columns);
						halfSpaceConstraints(c);
						for (int i = 0; i < (N+1); i++) {
							constraints[c_base+c_counter] = c[i];
							c_counter++;
						}
					}

				} else {
					// One single dimension, simply create the 2 half-spaces
					// Here we have 2 points in one single dimension
					constraint c1, c2;
					float p1, p2;
					for (int i = 0; i < N; i++) {
						c1[i] = PROJ(d,i);
						c2[i] = -PROJ(d,i);
						if (c1[i] != 0) {
							p1 = points[i][ic[0]];

							p2 = points[i][ic[1]];

							if (p1 > p2) {
								float temp = p1; p2 = p2; p2 = temp;
							}
						}
						const float treshold = (p1+p2)/2;
						c1[N] = (int)(p2+0.5) + treshold;						//<---- TODO: check this treshold
						c2[N] = -1*((int)(p1+0.5) + treshold);
					}
					for (int i = 0; i < (N+1); i++) {
						constraints[c_base+c_counter] = c1[i];
						c_counter++;
					}
					for (int i = 0; i < (N+1); i++) {
						constraints[c_base+c_counter] = c2[i];
						c_counter++;
					}
				}
			}
		}

		/*for (int i = 0; i < (K+1)*(N+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			echelon[ln][cl]=simplices[s_base+i];
		}
		
		int rank;
		columnEchelonForm(echelon, &rank, ic);
		ranks[idx] = rank;

		for (int i = 0; i < K+1; i++)
			independentCols[i+ic_base] = ic[i];

		for (int i = 0; i < (K+1)*(N+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			echelons[s_base+i] = echelon[ln][cl];
		}
		
		
		float c[N+1];
		int columns[K+1];
		for (int i = 0; i < K+1; i++) columns[i] = 1;
		getHyperplane(echelon, c, &nk[(nckRows-1)*(N+2)], rank, columns);

		for (int i = 0; i < N+1; i++) {
			constraints[idx][i] = c[i];
		}*/
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
void halfSpaceConstraints(float* const coefficients) {
	float b = 0;
	for (int i = 0; i < N; i++) {
		b += fabs(coefficients[i]);
	}
	b = b/2;
	coefficients[N] += b;
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
	while (index < rank) {
		if (base[it] != 0) {
			dimensions[index] = it;
			index++;
		}
		it++;
	}
	dimensions[rank] = N;		// homogeneous coordinates
	
	SMatrix m;

	// Initializes the first matrix (first row out) -> first coefficient
	for (int ln = 0; ln < rank; ln++) {
		for (int col = 0; col < rank; col++) {
			m[ln][col] = points[dimensions[ln+1]][col];
		}
	}
	c[dimensions[0]] = -1*determinant(m, rank);

	// updates the matrix -> more rank-1 coefficients
	for (int row = 0; row < rank-1; row++) {
		for (int col = 0; col < rank; col++) {
			m[row][col] = points[dimensions[row]][col];
		}
		int multiplier = ((row)%2)?(-1):(1);
		c[dimensions[row+1]] = determinant(m, rank)*multiplier;
	}

	for (int col = 0; col < rank; col++)
		m[rank-1][col] = points[dimensions[rank-1]][col];
	// last one is b
	c[N] = determinant(m, rank);
}


