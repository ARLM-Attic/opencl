#include "nchoosek.h"

// Returns the set of all permutations of 0..n-1 with k elements
// this is represented as a matrix with 'n choose k' lines and k columns
int** nchoosek(const int n, const int k, int& nck)
{
	nck = n;
	for (int i = 1; i < k; i++) {
		nck *= n-i;
		nck /= i;
	}
	nck /= k;

	int **res = new int*[nck];
	for (int i = 0; i < nck; i++) {
		res[i] = new int[k];
	}

	// First permutation
	for (int i = 0; i < k; i++) {
		res[0][i] = i;
	}

	for (int p = 1; p < nck; p++) {
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
	return res;
}

// This function returns a matrix with n+1 columns (the last element is the number of non-zero elements in the row)
//  and sum(i=1, i<=m, nchoosek(n, i)) rows.
// Each row contains 0s and 1s corresponds to a possible projection.
// Returns a matrix with n+2 columns and 'lines' lines, which represents all the possible projections
//  of 1 to m coordinates.
int** nchoosekMatrix(const int n, const int m, int& lines)
{
	// This counts sum(i = 1; i < m; nchoosek(n, i))
	lines = 0;
	for (int k = 1; k <= m; k++) {
		int comb = n;
		for (int i = 1; i < k; i++)
			comb *= n-i;
		for (int i = 2; i <= k; i++)
			comb /= i;
		lines += comb;
	}

	// Initializes the matrix
	int** res = new int*[lines];
	for (int i = 0; i < lines; i++) {
		res[i] =  new int[n+2];
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
		int** t = nchoosek(n, set, nck);
		for (int ln = 0; ln < nck; ln++) {
			for (int col = 0; col < set; col++) {
				res[pos][t[ln][col]] = 1;
			}
			res[pos][n+1] = set;
			pos++;
		}
		for (int i = 0; i < nck; i++)
			delete[] t[i];
		delete[] t;
	}

	return res;
}

// FIXME
// Returns a vector containing the nchoosekMatrix
// This vector size is lines*(n+2)
int* nchoosekVector(const int n, const int m, int &lines)
{
	int** nckm = nchoosekMatrix(n, m, lines);
	const int size = lines*(n+2);

	int* res = new int[size];
	for (int i = 0; i < size; i++) {
		const int idx = i/(n+2);
		const int idy = i%(n+2);
		res[i] = nckm[idx][idy];
	}

	for (int i = 0; i < lines; i++)
		delete[] nckm[i];
	delete[] nckm;

	return res;
}