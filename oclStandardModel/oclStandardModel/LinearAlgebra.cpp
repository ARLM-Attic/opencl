#include "LinearAlgebra.h"

/*
bool isZero(float value, float tolerance) {
	return (abs(value) < abs(tolerance));
}


void columnEchelonForm(Matrix matrix, int* rank, int* independentCols, const float tolerance) {
	*rank = 0;
	int rows = DIMENSION+1, cols = K+1;
	for (int i=0; i!=cols; ++i) {
		independentCols[i] = i;
	}
	int c = 0;
	int r = 0;
	while ((c != cols) && (r != rows)) {
		// Find value and index of largest element in the remainder of row r.
		int k = c;
		float max = abs(matrix[r][c]);
		for (int j=(c+1); j!=cols; ++j) {
			const float curr = abs(matrix[r][j]);
			if (max < curr) {
				k = j;
				max = curr;
			}
		}
		if (isZero(max, tolerance)) {
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

void columnEchelonForm(Matrix matrix, int* rank, int* independentCols) {
	columnEchelonForm(matrix, rank, independentCols, TOLERANCE);
}

float determinant(const Matrix matrix, int size, const float tolerance) {
	// TODO: implement the determinant decently
	if (size == 1) {
		return matrix[0][0];
	}
	else if (size == 2) {
		float x = matrix[0][0] * matrix[1][1];
		float y = matrix[1][0] * matrix[0][1];
		return x - y;
	}
	// temporary sarrus rule;
	else if (size == 3) {
		return matrix[0][0]*matrix[1][1]*matrix[2][2] + matrix[0][1]*matrix[1][2]*matrix[2][0] + matrix[1][0]*matrix[2][1]*matrix[0][2]
			- (matrix[0][2]*matrix[1][1]*matrix[2][0] + matrix[1][0]*matrix[0][1]*matrix[2][2] + matrix[0][0]*matrix[1][2]*matrix[2][1]);
	}
	else exit(_OCLPP_FAILURE);
}

float determinant(const Matrix matrix, int size) {
	return determinant(matrix, size, TOLERANCE);
}

// Pivot on matrix elements and returns the column index of the pivot element given a row index.
int pivot(Matrix m, const int row, const float tolerance) {
	int k = row;
	float temp, bigger = abs(m[row][row]);
	for (int r=(row+1); r!=DIMENSION+1; ++r) {							// <-------- check size
		if (bigger < (temp = abs(m[r][row]))) {
			k = r;
			bigger = temp;
		}
	}
	if (bigger < tolerance) {
		return -1;
	}
	if (k != row) {
		for (int c=0; c!=DIMENSION+1; ++c) {							// <-------- check size
			float sw = m[k][c];
			m[k][c] = m[row][c];
			m[row][c] = sw;
		}
		return k;
	}
	return 0;
}
*/
