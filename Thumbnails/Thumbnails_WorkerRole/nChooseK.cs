﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace RasterizerNamespace
{
    class nChooseK
    {
        // Returns the set of all permutations of 0..n-1 with k elements
        // this is represented as a matrix with 'n choose k' lines and k columns
        public static int[,] nchoosek(int n,  int k, out int nck) {
	        int comb = n;
	        for (int i = 1; i < k; i++)
		        comb *= n-i;
	        for (int i = 2; i <= k; i++)
		        comb /= i;
	
	        int[,] res = new int[comb,k];
	        
	        // First permutation
	        for (int i = 0; i < k; i++) {
		        res[0,i] = i;
	        }

	        for (int p = 1; p < comb; p++) {
		        // Copies the last combination
		        for (int i = 0; i < k; i++)
			        res[p, i] = res[p-1, i];
		
		        int e;
		        // From right to left, finds the first element to be changed
		        for (e = k-1; e >= 0; e--) {
			        int i = k-1-e;
			        if (res[p,e] < n-i-1)
				        break;
		        }
		        res[p,e]++;
		        // Updates the next elements
		        for (int i = e+1; i < k; i++)
			        res[p, i] = res[p, i-1] + 1;
	        }
	        nck = comb;
	        return res;
        }

        // This function returns a matrix with n+1 columns (the last element is the number of non-zero elements in the row)
        //  and sum(i=1, i<=m, nchoosek(n, i)) rows.
        // Each row contains 0s and 1s corresponds to a possible projection.
        // Returns a matrix with n+2 columns and 'lines' lines, which represents all the possible projections
        //  of 1 to m coordinates.
        public static int[,] getMatrix(int n, int m, out int lines) {
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
	        int[,] res = new int[count, n+2];
	        for (int i = 0; i < count; i++) {
		        for (int j = 0; j < n+2; j++) {
			        if (j == n)
				        res[i,j] = 1;
			        else
				        res[i,j] = 0;
		        }
	        }

	        int pos = 0;
	        for (int set = 1; set <= m; set++) {
		        int nck;
		        int[,] t = nchoosek(n, set, out nck);
		        for (int ln = 0; ln < nck; ln++) {
			        for (int col = 0; col < set; col++) {
				        res[pos, t[ln,col]] = 1;
			        }
			        res[pos, n+1] = set;
			        pos++;
		        }
	        }

	        lines = count;
	        return res;
        }

        // FIXME
        // Returns a vector containing the nchoosekMatrix
        // This vector size is lines*(n+2)
        public static int[] getVector(int n, int m, out int lines) {
	        int[,] nckm = getMatrix(n, m, out lines);
	        int size = (lines)*(n+2);
	        int[] res = new int[size];
	        for (int i = 0; i < size; i++) {
		        int idx = i/(n+2);
		        int idy = i%(n+2);
		        res[i] = nckm[idx, idy];
	        }

	        return res;
        }
    }
}
