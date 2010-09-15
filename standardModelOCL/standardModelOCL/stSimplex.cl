/* Leonardo Chatain
 * Date: 15/06/2010
 * Universidade Federal do Rio Grande do Sul
 */

#define PROJ_BASE (&(proj[(d)*(N+2)]))
#define PROJ(ln, col) (proj[(ln)*(N+2)+(col)])

/*****************************************************************************/

void loadMatrix(__global const float* simplices, __local smatrix simplex, const int idx);
void commitConstraint(__global float* global_ct, float* const local_ct);
void projectMatrix(__local const smatrix src, smatrix dst, __constant const int* const base);

void getHyperplaneFull(const smatrix points, float* const c);
void getHyperplaneFullN(const smatrix points, float* const c, int row);

int columnEchelonForm(smatrix matrix);

bool hasStandardOrientation(const float* c);
float treshold(const float* c);

/*****************************************************************************/

__kernel void stSimplex(__global const float* const simplices,
						__global float* const constraints,
						__constant const int* const proj,
						const int proj_rows)
{
	const int idx = get_global_id(0);
	const int local_idx = get_local_id(0);
	int ct_base = idx;

	//if (idx >= SIMPLICES * (N+1))
	if (idx >= SIMPLICES)
		return;

	// SHARED
	__local smatrix local_simplex[LOCAL_WORKSIZE];	// simplex shared cache
	// REGISTERS
	smatrix echelon[LOCAL_WORKSIZE];	// echelon form registers cache
	float ct[N+1];	// single constraint registers cache
	
	// Brings from global to shared all simplices (coalesced)
	loadMatrix(simplices, local_simplex[local_idx], idx);

	// Iterates through projections
	for (int d = 0; d < proj_rows; d++) {
		const int dim = PROJ(d,N+1);
		// Load simplex to echelon
		projectMatrix(local_simplex[local_idx], echelon, PROJ_BASE);
		// Compute echelon form
		const int rank = columnEchelonForm(echelon);

		if (rank == dim) {
			getHyperplaneFull(echelon, ct);
			const float bound = treshold(ct);
			if (!hasStandardOrientation(ct)) {			// REMOVE STANDARD ORIENTATION
				for (int i = 0; i < N+1; i++)
					ct[i] = -ct[i];
			}
			commitConstraint(&constraints[ct_base], ct);
			ct_base += SIMPLICES*(N+1);	// jumps (N+1) rows (constraints are stored in columns)
		}
		else {
			if (rank == dim+1) {
				// COPIES AGAIN THE MATRIX TO ECHELON (cache!!)
				projectMatrix(local_simplex[local_idx], echelon, PROJ_BASE);

				// find zero row
				int zero = 0;
				for (int i = 0; i < N; i++)
					if (PROJ_BASE[i] == 0) zero = i;
				// Computes the N constraints 
				getHyperplaneFullN(echelon, ct, zero);
				
				// COMMIT CONSTRAINTS (inside the function above)
				// UPDATE CT_BASE
			}
			else {
				// Find non-zero row
				int independent = 0;
				for (int i = 0; i < N; i++)
					if (PROJ_BASE[i] != 0) independent = i;
				// Find max and min of the row (dimension)
				float max_ = -INF_FLOAT, min_ = INF_FLOAT;
				for (int i = 0; i < K+1; i++) {
					const float val = local_simplex[local_idx][independent][i];
					if (max_ < val) max_ = val;
					if (min_ > val) min_ = val;
				}
				// Compute constraints
				ct[independent] = 1;
				const float b = -(min_ + max_) / 2;
				ct[N] = b;
				const float bound = treshold(ct) + b + min_;
				// compute the other constraint   ------>> LEANDRO

				// COMMIT CONSTRANTS
			}
		}
	}
}

/*****************************************************************************/

// Loads the simplex matrix
// Each thread loads ONE simplex
// The simplices vector is organized this way:
//	- #simplices floats corresponding to the first z
//	- #simplices floats corresponding to the second z
//	- #simplices floats corresponding to the third z
// To achieve maximum performance, #simplices MUST be a multiple of 16 (FIXME)
void loadMatrix(__global const float* simplices, __local smatrix simplex, const int idx) {
	const int sx = 180;
	simplex[0][0] = idx % sx;		// TODO: TOTALLY FIX ME!!! PLEASE
	simplex[1][0] = idx / (2 * (sx-1));
	if (idx % 2 == 0) {
		// p2
		simplex[0][1] = simplex[0][0] + 1.0f;
		simplex[1][1] = simplex[1][0];
		// p3
		simplex[0][2] = simplex[0][0] + 1.0f;
		simplex[1][2] = simplex[1][0] + 1.0f;
	} else {
		// p2
		simplex[0][1] = simplex[0][0] + 1.0f;
		simplex[1][1] = simplex[1][0] + 1.0f;
		// p3
		simplex[0][2] = simplex[1][0];
		simplex[1][2] = simplex[1][0] + 1.0f;
	}
	// coalescedly loads Z
	simplex[2][0] = simplices[idx];
	simplex[2][1] = simplices[idx + SIMPLICES];		// TODO: TOTALLY FIX ME!!!
	simplex[2][2] = simplices[idx + 2 * SIMPLICES];
	// homogeneous coordinates
	for (int i = 0; i < 3; i++) simplex[3][i] = 1.0f;
}

// Copies a constraint from local mem to global mem
// I'ts coalesced!!!
void commitConstraint(__global float* global_ct, float* const local_ct)
{
	for (int i = 0; i < N+1; i++) {
		global_ct[i*SIMPLICES] = local_ct[i];	// ASSUMING that SIMPLICES % 16 == 0
	}
}

/*****************************************************************************/

// Copies projected matrix to dst
void projectMatrix(__local const smatrix src, smatrix dst, __constant const int* const base) {
	for (int col = 0; col < K+1; col++) {
		for (int ln = 0; ln < N+1; ln++) {
			dst[ln][col] = base[ln]*src[ln][col];
		}
	}
}

/*****************************************************************************/

float treshold(const float* c)
{
	float acc = 0.0f;
	for (int i = 0; i < N; i++)
		acc += fabs(c[i]);
	return acc/2;
}

bool hasStandardOrientation(const float* c)
{
	for (int i = 0; i < N; i++) {
		if (c[i] > 0)
			return true;
		else if (c[i] < 0)
			return false;
	}
	return false;
}

/*****************************************************************************/

void getHyperplaneFull(const smatrix points, float* const c)
{
	c[0] = points[1][0]*points[2][1]*points[3][2] + points[1][1]*points[2][2]*points[3][0] + points[2][0]*points[3][1]*points[1][2]
		- (points[1][2]*points[2][1]*points[3][0] + points[2][0]*points[1][1]*points[3][2] + points[1][0]*points[2][2]*points[3][1]);
	c[1] = points[0][0]*points[2][1]*points[3][2] + points[0][1]*points[2][2]*points[3][0] + points[2][0]*points[3][1]*points[0][2]
		- (points[0][2]*points[2][1]*points[3][0] + points[2][0]*points[0][1]*points[3][2] + points[0][0]*points[2][2]*points[3][1]);
	c[2] = points[0][0]*points[1][1]*points[3][2] + points[0][1]*points[1][2]*points[3][0] + points[1][0]*points[3][1]*points[0][2]
		- (points[0][2]*points[1][1]*points[3][0] + points[1][0]*points[0][1]*points[3][2] + points[0][0]*points[1][2]*points[3][1]);
	c[3] = points[0][0]*points[1][1]*points[2][2] + points[0][1]*points[1][2]*points[2][0] + points[1][0]*points[2][1]*points[0][2]
		- (points[0][2]*points[1][1]*points[2][0] + points[1][0]*points[0][1]*points[2][2] + points[0][0]*points[1][2]*points[2][1]);

	c[1] *= -1;
	c[3] *= -1;
}

// Computes 3 constraints, discarding each column of the matrix ("row" is out)
void getHyperplaneFullN(const smatrix points, float* const c, const int row)
{
	if (row == 0) {
		// discards the first column
		c[0] = 0;
		c[1] = points[2][1]*points[3][2] - points[2][2]*points[3][1];
		c[2] = points[1][1]*points[3][2] - points[1][2]*points[3][1];
		c[3] = points[1][1]*points[2][2] - points[1][2]*points[2][1];
		
		// discards the second column
		c[4] = 0;
		c[5] = points[2][0]*points[3][2] - points[2][2]*points[3][0];
		c[6] = points[1][0]*points[3][2] - points[1][2]*points[3][0];
		c[7] = points[1][0]*points[2][2] - points[1][2]*points[2][0];

		// discards the third
		c[8] = 0;
		c[9] = points[2][0]*points[3][1] - points[2][1]*points[3][0];
		c[10] = points[1][0]*points[3][1] - points[1][1]*points[3][0];
		c[11] = points[1][0]*points[2][1] - points[1][1]*points[2][0];

		c[1] *= -1;
		c[3] *= -1;

		c[5] *= -1;
		c[7] *= -1;

		c[9] *= -1;
		c[11] *= -1;
	}
	else if (row == 1) {
		c[0] = points[2][1]*points[3][2] - points[2][2]*points[3][1];
		c[1] = 0;
		c[2] = points[0][1]*points[3][2] - points[0][2]*points[3][1];
		c[3] = points[0][1]*points[2][2] - points[0][2]*points[2][1];

		c[4] = points[2][0]*points[3][2] - points[2][2]*points[3][0];
		c[5] = 0;
		c[6] = points[0][0]*points[3][2] - points[0][2]*points[3][0];
		c[7] = points[0][0]*points[2][2] - points[0][2]*points[2][0];

		c[8] = points[2][0]*points[3][1] - points[2][1]*points[3][0];
		c[9] = 0;
		c[10] = points[0][0]*points[3][1] - points[0][1]*points[3][0];
		c[11] = points[0][0]*points[2][1] - points[0][1]*points[2][0];

		c[0] *= -1;
		c[3] *= -1;

		c[4] *= -1;
		c[7] *= -1;

		c[8] *= -1;
		c[11] *= -1;
	}
	else if (row == 2) {
		c[0] = points[1][1]*points[3][2] - points[1][2]*points[3][1];
		c[1] = points[0][1]*points[3][2] - points[0][2]*points[3][1];
		c[2] = 0;
		c[3] = points[0][1]*points[1][2] - points[0][2]*points[1][1];

		c[4] = points[1][0]*points[3][2] - points[1][2]*points[3][0];
		c[5] = points[0][0]*points[3][2] - points[0][2]*points[3][0];
		c[6] = 0;
		c[7] = points[0][0]*points[1][2] - points[0][2]*points[1][0];

		c[8] = points[1][0]*points[3][1] - points[1][1]*points[3][0];
		c[9] = points[0][0]*points[3][1] - points[0][1]*points[3][0];
		c[10] = 0;
		c[11] = points[0][0]*points[1][1] - points[0][1]*points[1][0];

		c[0] *= -1;
		c[3] *= -1;

		c[4] *= -1;
		c[7] *= -1;

		c[8] *= -1;
		c[11] *= -1;
	}
}

/*****************************************************************************/

bool isZero(const float value, const float tolerance)
{
	return (fabs(value) < fabs(tolerance));
}

int columnEchelonForm(smatrix matrix)
{
	int rank = 0;
	const int rows = N+1;
	const int cols = K+1;

	int c = 0;
	int r = 0;
	while ((c < cols) && (r < rows)) {
		// Find value and index of largest element in the remainder of row r.
		int k = c;
		float max_ = fabs(matrix[r][c]);
		for (int j = c + 1; j < cols; ++j) {
			const float curr = fabs(matrix[r][j]);
			if (max_ < curr) {
				k = j;
				max_ = curr;
			}
		}
		if (isZero(max_, TOLERANCE)) {
			// The row is negligible, zero it out.
			for (int j = c; j < cols; ++j) {
				matrix[r][j] = 0.0f;
			}
			++r;
		}
		else {
			float pivot;
			// Update rank.
			++rank;
			// Swap c-th and k-th columns.
			for (int j = 0; j < rows; ++j) {
				const float temp = matrix[j][c];
				matrix[j][c] = matrix[j][k];
				matrix[j][k] = temp;
			}

			// Divide the pivot column by the pivot element.
			pivot = 1.0f / matrix[r][c];
			for (int j = r; j < rows; ++j) {
				matrix[j][c] *= pivot;
			}

			// Subtract multiples of the pivot column from all the other columns.
			for (int j = 0; j < c; ++j) {
				pivot = matrix[r][j];
				for (int l = r; l < rows; ++l) {
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
	return rank;
}

/*****************************************************************************/


	// FOR DEBUG
	/*int out_id = 0;
	for (int i = 0; i < (K+1); i++) {
		for (int j = 0; j < (N+1); j++) {
			s_out[out_id + idx * (K+1)*(N+1)] = simplex[local_idx][i][j];
			out_id++;
		}
	}*/