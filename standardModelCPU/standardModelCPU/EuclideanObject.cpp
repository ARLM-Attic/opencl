#include "EuclideanObject.h"
#include <iostream>

void printMatrix(const Matrix m, int lines, int cols) {
	if (lines == 0) lines = N+1;
	if (cols == 0) cols = K+1;

	for (int ln = 0; ln < lines; ln++) {
		for (int col = 0; col < cols; col++) {
			cout << m[ln][col] << "\t";
		}
		cout << endl;
	}
	cout << endl;
}

// Returns the set of all permutations of 0..n-1 with k elements
// this is represented as a matrix with 'n choose k' lines and k columns
int** nchoosek(const int n, const int k, int* nck) {
	int comb = n;
	for (int i = 1; i < k; i++)
		comb *= n-i;
	for (int i = 2; i <= k; i++)
		comb /= i;
	
	int **res = (int **) malloc(sizeof(int*) * comb);
	for (int i = 0; i < comb; i++) {
		res[i] = (int *) malloc(sizeof(int) * k);
	}

	// First permutation
	for (int i = 0; i < k; i++) {
		res[0][i] = i;
	}

	for (int p = 1; p < comb; p++) {
		// Copies the last combination
		for (int i = 0; i < k; i++)
			res[p][i] = res[p-1][i];
		
		int e;
		// From right to left, finds the first element to be changed
		for (e = k-1; e >= 0; e--) {
			int i = k-1-e;
			if (res[p][e] < n-i-1)
				break;
		}
		res[p][e]++;
		// Updates the next elements
		for (int i = e+1; i < k; i++)
			res[p][i] = res[p][i-1] + 1;
	}
	*nck = comb;
	return res;
}

// This function returns a matrix with n+1 columns (the last element is the number of non-zero elements in the row)
//  and sum(i=1, i<=m, nchoosek(n, i)) rows.
// Each row contains 0s and 1s corresponds to a possible projection.
// Returns a matrix with n columns and 'lines' lines, which represents all the possible projections
//  of 1 to m coordinates.
int** nchoosekMatrix(const int n, const int m, int *lines) {
	// This counts sum(i = 1; i < m; nchoosek(n, i))
	int count = 0;
	for (int k = 1; k <= m; k++) {
		int comb = n;
		for (int i = 1; i < k; i++)
			comb *= n-i;
		for (int i = 2; i <= k; i++)
			comb /= i;
		count += comb;
	}

	// Initializes the matrix
	int** res = (int**) malloc(sizeof(int*)*count);
	for (int i = 0; i < count; i++) {
		res[i] = (int*) malloc(sizeof(int)*n+2);
		for (int j = 0; j < n+2; j++) {
			if (j == n)
				res[i][j] = 1;
			else
				res[i][j] = 0;
		}
	}

	int pos = 0;
	for (int set = 1; set <= m; set++) {
		int nck;
		int** t = nchoosek(n, set, &nck);
		for (int ln = 0; ln < nck; ln++) {
			for (int col = 0; col < set; col++) {
				res[pos][t[ln][col]] = 1;
			}
			res[pos][n+1] = set;
			pos++;
		}
		free(t);
	}

	*lines = count;
	return res;
}

bool hasStandardOrientation(const float* const coefficients) {
	for (int i = 0; i < N; i++) {
		if (coefficients[i] > 0)
			return true;
		else if (coefficients[i] < 0)
			return false;
	}
	return false;
}

void getHyperplane(const Matrix points, constraint* const c, const int* basis, const int rank, const int* columns) {
	// Guarantees the square matrix (it's not square because of the homogeneous coordinates)
	// The matrix must actually have rank columns and rank+1 rows
	if (rank != basis[N+1]) {
		cout << "rank != basis[N+1], exiting" << endl;
		system("pause");
		exit(FAILURE);
	}

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
	
	c->op = EQ;
	Matrix m;

	// Initializes the first matrix (first row out) -> first coefficient
	for (int ln = 0; ln < rank; ln++) {
		for (int col = 0; col < rank; col++) {
			m[ln][col] = points[dimensions[ln+1]][col];
		}
	}
	c->coefficients[dimensions[0]] = -1*determinant(m, rank);

	// updates the matrix -> more rank-1 coefficients
	for (int row = 0; row < rank-1; row++) {
		for (int col = 0; col < rank; col++) {
			m[row][col] = points[dimensions[row]][col];
		}
		int multiplier = ((row)%2)?(-1):(1);
		c->coefficients[dimensions[row+1]] = determinant(m, rank)*multiplier;
	}

	for (int col = 0; col < rank; col++)
		m[rank-1][col] = points[dimensions[rank-1]][col];
	// last one is b
	c->b = determinant(m, rank);
}

// Calculates the correct constraint for the given half-space
void halfSpaceConstraints(constraint* const halfSpace) {
	if (hasStandardOrientation(halfSpace->coefficients))
		halfSpace->op = LE;
	else
		halfSpace->op = LEQ;
	
	float b = 0;
	for (int i = 0; i < N; i++) {
		b += fabs(halfSpace->coefficients[i]);
	}
	b = b/2;
	halfSpace->b += b;
}

void copyProjectedMatrix(const Matrix src, Matrix dst, const int* basis) {
	for (int col = 0; col < K+1; col++) {
		for (int ln = 0; ln < N+1; ln++) {
			dst[ln][col] = basis[ln]*src[ln][col];
		}
	}
}

// Receives K+1 points (columns), each one with N+1 elements (lines, homogeneous coordinates)
vector<constraint*> stSimplex(const Matrix points) {
	vector<constraint*> v;
	Matrix echelon;
	// The highest projection
	int m = (N < K+1)?(N):(K+1);
	int rows;
	int** nk = nchoosekMatrix(N, m, &rows);

	// Iterates through all possible projections
	for (int d = 0; d < rows; d++) {
		const int dim = nk[d][N+1];
		// Copies the projected matrix to echelon
		copyProjectedMatrix(points, echelon, nk[d]);
		
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
			constraint *c = newConstraint();
			int columns[K+1];
			for (int i = 0; i < K+1; i++) columns[i] = 1;
			getHyperplane(echelon, c, nk[d], rank, columns);
			halfSpaceConstraints(c);
			v.push_back(c);
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
				constraint* c = newConstraint();
				getHyperplane(points, c, nk[d], rank, columns);
				halfSpaceConstraints(c);
				v.push_back(c);
				for (int i = 1; i < K+1; i++) {
					columns[i-1] = 1; columns[i] = 0;
					getHyperplane(points, c, nk[d], rank, columns);
					halfSpaceConstraints(c);
					v.push_back(c);
				}
			}
			else {
				// One single dimension, simply create the 2 half-spaces
				// Here we have 2 points in one single dimension
				constraint* c1 = newConstraint();
				constraint* c2 = newConstraint();
				float p1, p2;
				for (int i = 0; i < N; i++) {
					c1->coefficients[i] = nk[d][i];
					c2->coefficients[i] = -1*nk[d][i];
					if (nk[d][i] != 0) {
						p1 = points[i][independentCols[0]];
						p2 = points[i][independentCols[1]];
						if (p1 > p2) {	//swap them
							float temp = p1; p2 = p2; p2 = temp;
						}
					}
					const float treshold = (p1+p2)/2;
					c1->b = (int)(p2+0.5) + treshold;								//<---- TODO: check this treshold
					c2->b = -1*((int)(p1+0.5) + treshold);
				}
				v.push_back(c1);
				v.push_back(c2);
			}
		}
	}
	free(nk);
	return v;
}