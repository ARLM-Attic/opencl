//////////////////////////////////////////////////////////////////////////////

void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols);
float determinant(const SMatrix matrix, int size);

bool hasStandardOrientation(const float* const coefficients);
void halfSpaceConstraints(float* const coefficients);

void copyProjectedMatrix(SMatrix src, SMatrix dst, __global const int* basis);

void getHyperplane(SMatrix points, float* const c, __global const int* basis, const int rank, const int* columns);

//////////////////////////////////////////////////////////////////////////////

__kernel void stSimplex(__global int* points,
						__global CMatrix constraints,
						__global const int* nk,
						__global const int nckRows)
{
	int idx = get_global_id(0);
	if (idx < SIMPLICES*(N+1)) {
		SMatrix echelon;

		// Iterates through all possible projections
		for (int d = 0; d < nckRows; d++) {
			const int dim = nk[d*(K+2)+(N+1)]; // nk[d][N+1]
			// Copies the projected matrix to echelon
			copyProjectedMatrix(*(SMatrix*)&points[idx*(N+1)][0], echelon, &nk[d*(N+1)]);
			
			// Compute matrix representation of the flat induced by the vertices
			// projected onto current d-dimensional space (i.e., the reduced
			// column echelon form of vertices_proj) and its dimensionality
			// (i.e., the rank of flat's matrix).
			int rank;
			int independentCols[K+1];
			columnEchelonForm(echelon, &rank, independentCols);

			// Compute constraints from induced flat if it is an hyperplane in
			// current d-dimensional space.
			if (rank == dim) {	// the last element is the dimension of the projection
				Constraint c;
				int columns[K+1];
				for (int i = 0; i < K+1; i++) columns[i] = 1;
				getHyperplane(echelon, c, &nk[d*(N+1)], rank, columns);
				halfSpaceConstraints(c);
				//TODO: put the constraint in the list
			}
			// Compute constraints from extrusion only if projected vertices
			// induce current d-dimensional space.
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
					Constraint c;
					getHyperplane(*(SMatrix*)&points[idx*(N+1)][0], c, &nk[d*(N+1)], rank, columns); // TODO: perigo!
					halfSpaceConstraints(c);
					//TODO: put the constraint in the list
					for (int i = 1; i < K+1; i++) {
						columns[i-1] = 1; columns[i] = 0;
						getHyperplane(*(SMatrix*)&points[idx*(N+1)][0], c, &nk[d*(N+1)], rank, columns);
						halfSpaceConstraints(c);
						//TODO: put the constraint in the list
					}
				}
				else {
					// One single dimension, simply create the 2 half-spaces
					// Here we have 2 points in one single dimension
					Constraint c1;
					Constraint c2;
					float p1, p2;
					for (int i = 0; i < N; i++) {
						c1[i] = nk[d*(K+2)+i];
						c2[i] = -1*nk[d*(K+2)+i];
						if (nk[d*(K+2)+i] != 0) {
							p1 = points[i][independentCols[0]];
							p2 = points[i][independentCols[1]];
							if (p1 > p2) {	//swap them
								float temp = p1; p2 = p2; p2 = temp;
							}
						}
						const float treshold = (p1+p2)/2;
						c1[N] = (int)(p2+0.5) + treshold;						//<---- TODO: check this treshold
						c2[N] = -1*((int)(p1+0.5) + treshold);
					}
					//TODO: put the constraint in the list
					//TODO: put the constraint in the list
				}
			}
		}
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

void copyProjectedMatrix(SMatrix src,  SMatrix dst, __global const int* basis) {
	for (int col = 0; col < K+1; col++) {
		for (int ln = 0; ln < N+1; ln++) {
			dst[ln][col] = basis[ln]*src[ln][col];
		}
	}
}

bool isZero(float value, float tolerance) {
	return (fabs(value) < fabs(tolerance));
}

void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols) {
	*rank = 0;
	int rows = N+1, cols = K+1;
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
				float temp = matrix[j][c];
				matrix[j][c] = matrix[j][k];
				matrix[j][k] = temp;
			}
			// Swap independentCols[c] and independentCols[k]
			int temp = independentCols[c];
			independentCols[c] = independentCols[k];
			independentCols[k] = temp;
			// Divide the pivot column by the pivot element.
			pivot = 1.0 / matrix[r][c];
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
float determinant(const SMatrix matrix, int size) {
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

void getHyperplane(SMatrix points, float* const c, __global const int* basis, const int rank, const int* columns) {
	// Guarantees the square matrix (it's not square because of the homogeneous coordinates)
	// The matrix must actually have rank columns and rank+1 rows
	
	int dimensions[N+1];
	int index = 0;
	int it = 0;
	// Fills the dimensions array
	while (index < rank) {
		if (basis[it] != 0) {
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

