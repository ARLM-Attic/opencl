void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols);

//////////////////////////////////////////////////////////////////////////////

__kernel void stSimplex(__global float* simplices,
						__global float* echelons,
						__global int* ranks,
						__global int* independentCols,
						__global CMatrix constraints,
						__constant const int* nk,
						__constant const int nckRows)
{
	const int idx = get_global_id(0);
	const int s_base = (N+1)*(K+1)*idx;
	const int ic_base = (K+1)*idx;
	SMatrix echelon;
	int ic[K+1];

	if (idx < SIMPLICES) {
		for (int i = 0; i < (K+1)*(N+1); i++) {
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
	}
}

//////////////////////////////////////////////////////////////////////////////

bool isZero(float value, float tolerance) {
	return (fabs(value) < fabs(tolerance));
}

void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols) {
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