using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Rasterizer
{
    class Rasterizer
    {
        private static readonly int N_DIMENSIONS = 3;
        private static readonly int K = 2;
        private static readonly int C_PER_SIMPLEX = 16;

        private int numSimplices;
        private int sSize;
        private float[] simplices;

        private int cSize;
        private float[] constraints;

        private int[] nckv;
        private int nckRows;
        private int nck_size;

        //SMatrix = float[N_DIMENSIONS+1,K+1]
        //constraint = float[N_DIMENSIONS+1];

        // METHODS:

        public Rasterizer()
        {
            int _m_ = (N_DIMENSIONS < K + 1) ? (N_DIMENSIONS) : (K + 1);
            nckv = nchoosekVector(N_DIMENSIONS, _m_, &nckRows);
            nck_size = (N_DIMENSIONS + 2) * nckRows * sizeof(int);
            constraints = null;
        }

        //private void columnEchelonForm(SMatrix matrix, int* rank, int* independentCols);
		private void echelonTest(int* ranks, int* indCols);
		//private void makeStandardOriented(float* const coefficients);
		//private void triangleFaceOrientation(const SMatrix points, const int rank,
		//							const int* const columns, float* const coefficients);
		//private void getHyperplane(SMatrix points, float* const c, const int* base, const int rank, const int* columns);
		private void halfSpaceConstraints(float[] coefficients);

		private bool isZero(float value, float tolerance);
		//private float determinant(const SMatrix matrix, int size);
		//private void copyProjectedMatrix(float[,] src,  float[,] dst, const int* base);

        public void stSimplex() {
	        for (int idx = 0; idx < numSimplices; idx++) {
		        int s_base = (N_DIMENSIONS+1)*(K+1)*idx;
		        int ic_base = (K+1)*idx;
		        int c_base = (N_DIMENSIONS+1)*C_PER_SIMPLEX*idx;

		        int c_counter = 0;

		        float[,] echelon = new float[N_DIMENSIONS+1,K+1];
                float[,] points = new float[N_DIMENSIONS+1,K+1];

		        // Load matrix into registers
		        for (int i = 0; i < (K+1)*(N_DIMENSIONS+1); i++) {
			        int ln = i/(K+1);
			        int cl = i%(K+1);
			        points[ln,cl] = simplices[idx*(K+1) + ln*numSimplices*(K+1) + cl];
		        }

		        // Iterates through all possible projections
		        for (int d = 0; d < nckRows; d++) {
                    const int dim = nckv[(d) * (N_DIMENSIONS + 2) + (N_DIMENSIONS + 1)];
                    //int[] proj_base = new int[XXXXXXXXX];
                    //copiar
			        const int* proj_base = &nckv[(d)*(N_DIMENSIONS+2)+(0)];

			        //Copies the projected matrix to echelon
			        copyProjectedMatrix(points, echelon, proj_base);

			        // Compute matrix representation of the flat induced by the vertices
			        // projected onto current d-dimensional space (i.e., the reduced
			        // column echelon form of vertices_proj) and its dimensionality
			        // (i.e., the rank of flat's matrix).
                    int rank;
                    int[] ic = new int[K + 1];
			        columnEchelonForm(echelon, &rank, ic);

			        // Compute constraints from induced flat if it is an hyperplane in
			        // current d-dimensional space.
			        if (rank == dim) {
                        float[] c = new float[N_DIMENSIONS+1];
				        int[] columns = new int[K+1];
				        for (int i = 0; i < K+1; i++)
                            columns[i] = 1;
				        
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
					        int[] columns = new int[K+1];
					        for (int i = 1; i < K+1; i++) columns[i] = 1;
					        columns[0] = 0;
					        float[] c = new float[N_DIMENSIONS+1];

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
						        minC = Math.Min(minC, points[coord,p]);
						        maxC = Math.Max(maxC, points[coord,p]);
					        }

					        float[] consMin = new float[N_DIMENSIONS+1];
					        float[] consMax = new float[N_DIMENSIONS+1];

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
    }
}
