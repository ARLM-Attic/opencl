using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using System.Globalization;
using System.Threading.Tasks;

using System.Diagnostics;

namespace RasterizerNamespace
{
    class Rasterizer
    {
        private static readonly int N_DIMENSIONS = 3;
        private static readonly int K = 2;
        private static readonly int C_PER_SIMPLEX = 16;
        private static readonly float TOLERANCE = 1.0e-5f;

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
            nckv = nChooseK.getVector(N_DIMENSIONS, _m_, out nckRows);
            nck_size = (N_DIMENSIONS + 2) * nckRows * sizeof(int);
            constraints = null;
        }

        public void initializeConstraints() {
	        constraints = new float[cSize];
	        // Initialize the constraints so that evaluation is always true (no constraint)
	        for (int c = 0; c < numSimplices*C_PER_SIMPLEX*(N_DIMENSIONS+1); c++) {
		        constraints[c] = 0;
	        }
        }

        public float[] loadDataset(Stream file)
        {
            StreamReader sr = new StreamReader(file);
            char[] separators = new char [] {' '};

            string line = sr.ReadLine();
            string[] numbers = line.Split(separators, StringSplitOptions.RemoveEmptyEntries);

	        int sx = Convert.ToInt32(numbers[0]), sy = Convert.ToInt32(numbers[1]);
	        float[,] heights = new float[sx, sy];

	        for (int row = 0; row < sx; row++) {
                line = sr.ReadLine();
                numbers = line.Split(separators, StringSplitOptions.RemoveEmptyEntries);
		        for (int col = 0; col < sy; col++) {
                    heights[row, col] = (float) Convert.ToDouble(numbers[col], CultureInfo.InvariantCulture.NumberFormat);
		        }
	        }

	        numSimplices = sx*sy*2 ;
	        int size = numSimplices  * (K+1) * (N_DIMENSIONS+1); // x1, x2, x3, 1
	        float[] simplices = new float[size];

	        for (int i=0; i<size; i++)
		        simplices[i] = 0;

	        int ns = numSimplices * (K+1);
	        int idx = 0;
	        for (int y = 0; y < sy - 1; y++) {
		        for (int x = 0; x < sx - 1; x++) {
			        // Even - 1st triangle
			        simplices[idx] = x; // 1st coord, 1st point
			        simplices[idx + ns] = y; // 2nd coord, 1st point
			        simplices[idx + 2*ns] = heights[x, y]; // 3rd coord, 1st point
			        simplices[idx + 3*ns] = 1; //homogenous coord, 1st point
			        idx++;

			        simplices[idx] = x+1; // 1st coord, 2nd point
			        simplices[idx + ns] = y; // 2nd coord, 2nd point
			        simplices[idx + 2*ns] = heights[x+1, y]; // 3rd coord, 2nd point
			        simplices[idx + 3*ns] = 1; //homogenous coord, 2nd point
			        idx++;

			        simplices[idx] = x+1; // 1st coord, 3rd point
			        simplices[idx + ns] = y+1; // 2nd coord, 3rd point
			        simplices[idx + 2*ns] = heights[x+1, y+1]; // 3rd coord, 3rd point
			        simplices[idx + 3*ns] = 1; //homogenous coord, 3rd point
			        idx++;

			        // Odd - 2nd triangle
			        simplices[idx] = x; // 1st coord, 1st point
			        simplices[idx + ns] = y; // 2nd coord, 1st point
			        simplices[idx + 2*ns] = heights[x, y]; // 3rd coord, 1st point
			        simplices[idx + 3*ns] = 1; //homogenous coord, 1st point
			        idx++;

			        simplices[idx] = x+1; // 1st coord, 2nd point
			        simplices[idx + ns] = y+1; // 2nd coord, 2nd point
			        simplices[idx + 2*ns] = heights[x+1, y+1]; // 3rd coord, 2nd point
			        simplices[idx + 3*ns] = 1; //homogenous coord, 2nd point
			        idx++;

			        simplices[idx] = x; // 1st coord, 3rd point
			        simplices[idx + ns] = y+1; // 2nd coord, 3rd point
			        simplices[idx + 2*ns] = heights[x, y+1]; // 3rd coord, 3rd point
			        simplices[idx + 3*ns] = 1; //homogenous coord, 3rd point
			        idx++;
		        }
	        }

	        return simplices;
        }

        public void readHeightMap(Stream file)
        {
            simplices = loadDataset(file);
            cSize = (N_DIMENSIONS + 1) * numSimplices * C_PER_SIMPLEX;
            sSize = (N_DIMENSIONS + 1) * (K + 1) * numSimplices;
        }

        public void readTriangles(Stream file)
        {
            StreamReader sr = new StreamReader(file);

            string line = sr.ReadLine();

            char[] separators = new char [] {' '};
            string[] numbers = line.Split(separators, StringSplitOptions.RemoveEmptyEntries);

            numSimplices = Convert.ToInt32(numbers[0]);

            cSize = (N_DIMENSIONS + 1) * numSimplices * C_PER_SIMPLEX;
            sSize = (N_DIMENSIONS + 1) * (K + 1) * numSimplices;
            simplices = new float[sSize];
            //Read the file
            //Simplex s, point p, coordinate dim
            for (int s = 0; s < numSimplices; s++)
            {
                for (int dim = 0; dim < N_DIMENSIONS; dim++)
                {
                    do
                    {
                        line = sr.ReadLine();
                    } while (line == "");

                    numbers = line.Split(separators, StringSplitOptions.RemoveEmptyEntries);
                    for (int p = 0; p < K + 1; p++)
                    {
                        simplices[dim * (numSimplices) * (K + 1) + s * (K + 1) + p] = (float)Convert.ToDouble(numbers[p], CultureInfo.InvariantCulture.NumberFormat);
                        //homogenous coordinates
                        simplices[N_DIMENSIONS * (numSimplices) * (K + 1) + s * (K + 1) + p] = 1;
                    }
                }
            }
        }

        public void readInput(Stream file, bool isHeightMap)
        {
	        if(isHeightMap)
		        readHeightMap(file);
	        else
		        readTriangles(file);
        }

        private void columnEchelonForm(float[,] matrix, out int rank, ref int[] independentCols)
        {
            rank = 0;
            int rows = N_DIMENSIONS + 1;
            int cols = K + 1;
            for (int i = 0; i != cols; ++i)
            {
                independentCols[i] = i;
            }

            int c = 0;
            int r = 0;
            while ((c != cols) && (r != rows))
            {
                // Find value and index of largest element in the remainder of row r.
                int k = c;
                float max = Math.Abs(matrix[r, c]);
                for (int j = (c + 1); j != cols; ++j)
                {
                    float curr = Math.Abs(matrix[r, j]);
                    if (max < curr)
                    {
                        k = j;
                        max = curr;
                    }
                }
                if (isZero(max, TOLERANCE))
                {
                    // The row is negligible, zero it out.
                    for (int j = c; j != cols; ++j)
                    {
                        matrix[r, j] = 0;
                    }
                    r++;
                }
                else
                {
                    float pivot;
                    // Update rank.
                    rank++;
                    // Swap c-th and k-th columns.
                    for (int j = 0; j != rows; ++j)
                    {
                        float temp = matrix[j, c];
                        matrix[j, c] = matrix[j, k];
                        matrix[j, k] = temp;
                    }

                    // Swap independentCols[c] and independentCols[k]
                    int tempIC = independentCols[c];
                    independentCols[c] = independentCols[k];
                    independentCols[k] = tempIC;
                    // Divide the pivot column by the pivot element.
                    pivot = 1.0f / matrix[r, c];
                    for (int j = r; j != rows; ++j)
                    {
                        matrix[j, c] *= pivot;
                    }
                    // Subtract multiples of the pivot column from all the other columns.
                    for (int j = 0; j != c; ++j)
                    {
                        pivot = matrix[r, j];

                        for (int l = r; l != rows; ++l)
                        {
                            matrix[l, j] -= (pivot * matrix[l, c]);
                        }
                    }

                    for (int j = c + 1; j != cols; ++j)
                    {
                        pivot = matrix[r, j];
                        for (int l = r; l != rows; ++l)
                        {
                            matrix[l, j] -= (pivot * matrix[l, c]);
                        }
                    }
                    c++;
                    r++;
                }
            }
        }

		private void echelonTest(int[] ranks, int[] indCols) {
	        for (int i = 0; i < numSimplices; i++) {
		        float[,] e = new float[N_DIMENSIONS+1, K+1];
		        int s_base = (N_DIMENSIONS+1)*(K+1)*i;
		        //copy matrix
		        for (int j = 0; j < (N_DIMENSIONS+1)*(K+1); j++) {
			        int ln = j/(K+1);
			        int cl = j%(K+1);
			        e[ln, cl]=simplices[s_base+j];
		        }
		        // echelon form
		        int[] ic = new int[K+1];
		        columnEchelonForm(e, out ranks[i], ref ic);
		        int ic_base = i*(K+1);
		        for (int j = 0; j < (K+1); j++)
			        indCols[ic_base+i] = ic[j];
		        //copy back
		        for (int j = 0; j < (N_DIMENSIONS+1)*(K+1); j++) {
			        int ln = j/(K+1);
			        int cl = j%(K+1);
			        simplices[s_base+j] = e[ln, cl];
		        }
	        }
        }

        private void makeStandardOriented(ref float[] coefficients)
        {
            bool hasStdOrientation = true;
            for (int i = 0; i <= N_DIMENSIONS; i++)
                if (coefficients[i] < -TOLERANCE)
                {
                    hasStdOrientation = false;
                    break;
                }

            if (!hasStdOrientation)
                for (int i = 0; i <= N_DIMENSIONS; i++)
                    coefficients[i] = -coefficients[i];
        }

        private void triangleFaceOrientation(float[,] points, int rank, int[] columns, float[] coefficients)
        {
            for (int p = 0; p < rank; p++)
                if (columns[p] == 0)
                {
                    float dot = 0;
                    for (int coord = 0; coord <= N_DIMENSIONS; coord++)
                        dot += coefficients[coord] * points[coord, p];

                    if (dot > 0)
                    {
                        for (int i = 0; i < N_DIMENSIONS + 1; i++)
                            coefficients[i] = -coefficients[i];
                        break;
                    }
                }
        }

		private void getHyperplane(float[,] points, ref float[] c, int[] base_, int rank, int[] columns) {
	        // Guarantees the square matrix (it's not square because of the homogeneous coordinates)
	        // The matrix must actually have rank columns and rank+1 rows
	        if (rank != base_[N_DIMENSIONS+1]) {
                return;
	        }

	        int[] dimensions = new int[N_DIMENSIONS+1];
	        int index = 0;
	        int it = 0;
	        // Fills the dimensions array
	        for(int i=0; i<N_DIMENSIONS+1; i++)
		        c[i] = 0;

	        while (index < rank) {
		        if (base_[it] != 0) {
			        dimensions[index] = it;
			        index++;
		        }
		        it++;
	        }
	        dimensions[rank] = N_DIMENSIONS;		// homogeneous coordinates

	        float[,] m = new float[N_DIMENSIONS+1, K+1];

	        // Initializes the first matrix (first row out) -> first coefficient
	        int nCols;
	        for (int ln = 0; ln < rank; ln++) {
		        nCols = 0;
		        int col = 0;

		        while(nCols<rank) {
			        if(columns[col]!=0)
				        m[ln, nCols++] = points[dimensions[ln+1], col];
			        col++;
		        }
	        }
	        int sig = ((1 + rank+1)&1)!=0 ? -1 : 1;
	        c[dimensions[0]] = sig * determinant(m, rank);

	        // updates the matrix -> more rank-1 coefficients
	        for (int row = 0; row < rank-1; row++) {
		        nCols = 0;
		        int col=0;

		        while(nCols<rank) {
			        if(columns[col]!=0)
				        m[row, nCols++] = points[dimensions[row], col];
			        col++;
		        }
		        int multiplier = (((row+1+1) + nCols+1)&1)!=0 ? -1 : 1;
		        c[dimensions[row+1]] = determinant(m, nCols)*multiplier;
	        }

	        nCols = 0;
	        int col2=0;

	        while(nCols<rank) {
		        if(columns[col2]!=0)
			        m[rank-1, nCols++] = points[dimensions[rank-1], col2];
		        col2++;
	        }

	        sig = ((rank+1 + nCols+1)&1)!=0 ? -1 : 1;
	        c[N_DIMENSIONS] = sig*determinant(m, nCols);
        }

        private void halfSpaceConstraints(ref float[] coefficients)
        {
            float b = 0;
            for (int i = 0; i < N_DIMENSIONS; i++)
            {
                b += Math.Abs(coefficients[i]);
            }
            b = b / 2;
            coefficients[N_DIMENSIONS] -= b;
        }

        private bool isZero(float value, float tolerance)
        {
            return (Math.Abs(value) < Math.Abs(tolerance));
        }

        private float determinant(float[,] matrix, int size)
        {
            if (size == 1)
            {
                return matrix[0, 0];
            }
            else if (size == 2)
            {
                float x = matrix[0, 0] * matrix[1, 1];
                float y = matrix[1, 0] * matrix[0, 1];
                return x - y;
            }
            // Sarrus
            else if (size == 3)
            {
                return matrix[0, 0] * matrix[1, 1] * matrix[2, 2] + matrix[0, 1] * matrix[1, 2] * matrix[2, 0] + matrix[1, 0] * matrix[2, 1] * matrix[0, 2]
                - (matrix[0, 2] * matrix[1, 1] * matrix[2, 0] + matrix[1, 0] * matrix[0, 1] * matrix[2, 2] + matrix[0, 0] * matrix[1, 2] * matrix[2, 1]);
            }
            else
                return float.NaN;
        }

        private void copyProjectedMatrix(float[,] src, float[,] dst, int[] base_)
        {
            for (int col = 0; col < K + 1; col++)
            {
                for (int ln = 0; ln < N_DIMENSIONS + 1; ln++)
                {
                    dst[ln, col] = base_[ln] * src[ln, col];
                }
            }
        }

        public void stSimplex()
        {
            for (int idx = 0; idx < numSimplices; idx++) {
                int s_base = (N_DIMENSIONS + 1) * (K + 1) * idx;
                int ic_base = (K + 1) * idx;
                int c_base = (N_DIMENSIONS + 1) * C_PER_SIMPLEX * idx;

                int c_counter = 0;

                float[,] echelon = new float[N_DIMENSIONS + 1, K + 1];
                float[,] points = new float[N_DIMENSIONS + 1, K + 1];

                // Load matrix into registers
                for (int i = 0; i < (K + 1) * (N_DIMENSIONS + 1); i++)
                {
                    int ln = i / (K + 1);
                    int cl = i % (K + 1);
                    points[ln, cl] = simplices[idx * (K + 1) + ln * numSimplices * (K + 1) + cl];
                }

                // Iterates through all possible projections
                for (int d = 0; d < nckRows; d++)
                {
                    int dim = nckv[(d) * (N_DIMENSIONS + 2) + (N_DIMENSIONS + 1)];
                    //copiar
                    int[] proj_base = new int[N_DIMENSIONS + 2];
                    for (int pbI = 0; pbI < N_DIMENSIONS + 2; pbI++)
                        proj_base[pbI] = nckv[(d) * (N_DIMENSIONS + 2) + (pbI)];

                    //Copies the projected matrix to echelon
                    copyProjectedMatrix(points, echelon, proj_base);

                    // Compute matrix representation of the flat induced by the vertices
                    // projected onto current d-dimensional space (i.e., the reduced
                    // column echelon form of vertices_proj) and its dimensionality
                    // (i.e., the rank of flat's matrix).
                    int rank;
                    int[] ic = new int[K + 1];
                    columnEchelonForm(echelon, out rank, ref ic);

                    // Compute constraints from induced flat if it is an hyperplane in
                    // current d-dimensional space.
                    if (rank == dim)
                    {
                        float[] c = new float[N_DIMENSIONS + 1];
                        int[] columns = new int[K + 1];
                        for (int i = 0; i < K + 1; i++)
                            columns[i] = 1;

                        getHyperplane(echelon, ref c, proj_base, rank, columns);
                        makeStandardOriented(ref c);
                        halfSpaceConstraints(ref c);
                        for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                        {
                            constraints[c_base + c_counter] = c[i];
                            c_counter++;
                        }

                    }
                    else if (rank == dim + 1)
                    {
                        if (dim > 1)
                        {
                            // Dimension is higher than 1,
                            // create constraints from the extrusion of half-spaces bounded
                            // by the facets of the convex polytope.
                            // *NOTE*: here we assume that there are only triangles (2-simplex)
                            // So, now we must create 3 new constraints, relative to facets 1-2, 1-3, 2-3
                            int[] columns = new int[K + 1];
                            for (int i = 1; i < K + 1; i++) columns[i] = 1;
                            columns[0] = 0;
                            float[] c = new float[N_DIMENSIONS + 1];

                            getHyperplane(points, ref c, proj_base, rank - 1, columns);
                            triangleFaceOrientation(points, rank, columns, c);
                            halfSpaceConstraints(ref c);
                            for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                            {
                                constraints[c_base + c_counter] = c[i];
                                c_counter++;
                            }

                            for (int i = 1; i < K + 1; i++)
                            {
                                columns[i - 1] = 1; columns[i] = 0;
                                getHyperplane(points, ref c, proj_base, rank - 1, columns);
                                triangleFaceOrientation(points, rank, columns, c);
                                halfSpaceConstraints(ref c);
                                for (int j = 0; j < (N_DIMENSIONS + 1); j++)
                                {
                                    constraints[c_base + c_counter] = c[j];
                                    c_counter++;
                                }
                            }

                        }
                        else
                        {
                            float minC;
                            float maxC;
                            int coord = 0;

                            for (int i = 0; i < N_DIMENSIONS; i++)
                                if (proj_base[i] != 0)
                                    coord = i;

                            minC = float.MaxValue;
                            maxC = float.MinValue;
                            for (int p = 0; p < N_DIMENSIONS; p++)
                            {
                                minC = Math.Min(minC, points[coord, p]);
                                maxC = Math.Max(maxC, points[coord, p]);
                            }

                            float[] consMin = new float[N_DIMENSIONS + 1];
                            float[] consMax = new float[N_DIMENSIONS + 1];

                            for (int i = 0; i < N_DIMENSIONS; i++)
                            {
                                if (i == coord)
                                    consMin[i] = -1;
                                else
                                    consMin[i] = 0;
                            }
                            consMin[N_DIMENSIONS] = minC;

                            for (int i = 0; i < N_DIMENSIONS; i++)
                            {
                                if (i == coord)
                                    consMax[i] = 1;
                                else
                                    consMax[i] = 0;
                            }
                            consMax[N_DIMENSIONS] = -maxC;

                            halfSpaceConstraints(ref consMin);
                            halfSpaceConstraints(ref consMax);

                            for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                            {
                                constraints[c_base + c_counter] = consMin[i];
                                c_counter++;
                            }

                            for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                            {
                                constraints[c_base + c_counter] = consMax[i];
                                c_counter++;
                            }
                        }
                    }
                }
            }
        }

        public void stSimplexParallel()
        {
            Parallel.For(0, numSimplices, idx =>
            {
                int s_base = (N_DIMENSIONS + 1) * (K + 1) * idx;
                int ic_base = (K + 1) * idx;
                int c_base = (N_DIMENSIONS + 1) * C_PER_SIMPLEX * idx;

                int c_counter = 0;

                float[,] echelon = new float[N_DIMENSIONS + 1, K + 1];
                float[,] points = new float[N_DIMENSIONS + 1, K + 1];

                // Load matrix into registers
                for (int i = 0; i < (K + 1) * (N_DIMENSIONS + 1); i++)
                {
                    int ln = i / (K + 1);
                    int cl = i % (K + 1);
                    points[ln, cl] = simplices[idx * (K + 1) + ln * numSimplices * (K + 1) + cl];
                }

                // Iterates through all possible projections
                for (int d = 0; d < nckRows; d++)
                {
                    int dim = nckv[(d) * (N_DIMENSIONS + 2) + (N_DIMENSIONS + 1)];
                    //copiar
                    int[] proj_base = new int[N_DIMENSIONS + 2];
                    for (int pbI = 0; pbI < N_DIMENSIONS + 2; pbI++)
                        proj_base[pbI] = nckv[(d) * (N_DIMENSIONS + 2) + (pbI)];

                    //Copies the projected matrix to echelon
                    copyProjectedMatrix(points, echelon, proj_base);

                    // Compute matrix representation of the flat induced by the vertices
                    // projected onto current d-dimensional space (i.e., the reduced
                    // column echelon form of vertices_proj) and its dimensionality
                    // (i.e., the rank of flat's matrix).
                    int rank;
                    int[] ic = new int[K + 1];
                    columnEchelonForm(echelon, out rank, ref ic);

                    // Compute constraints from induced flat if it is an hyperplane in
                    // current d-dimensional space.
                    if (rank == dim)
                    {
                        float[] c = new float[N_DIMENSIONS + 1];
                        int[] columns = new int[K + 1];
                        for (int i = 0; i < K + 1; i++)
                            columns[i] = 1;

                        getHyperplane(echelon, ref c, proj_base, rank, columns);
                        makeStandardOriented(ref c);
                        halfSpaceConstraints(ref c);
                        for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                        {
                            constraints[c_base + c_counter] = c[i];
                            c_counter++;
                        }

                    }
                    else if (rank == dim + 1)
                    {
                        if (dim > 1)
                        {
                            // Dimension is higher than 1,
                            // create constraints from the extrusion of half-spaces bounded
                            // by the facets of the convex polytope.
                            // *NOTE*: here we assume that there are only triangles (2-simplex)
                            // So, now we must create 3 new constraints, relative to facets 1-2, 1-3, 2-3
                            int[] columns = new int[K + 1];
                            for (int i = 1; i < K + 1; i++) columns[i] = 1;
                            columns[0] = 0;
                            float[] c = new float[N_DIMENSIONS + 1];

                            getHyperplane(points, ref c, proj_base, rank - 1, columns);
                            triangleFaceOrientation(points, rank, columns, c);
                            halfSpaceConstraints(ref c);
                            for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                            {
                                constraints[c_base + c_counter] = c[i];
                                c_counter++;
                            }

                            for (int i = 1; i < K + 1; i++)
                            {
                                columns[i - 1] = 1; columns[i] = 0;
                                getHyperplane(points, ref c, proj_base, rank - 1, columns);
                                triangleFaceOrientation(points, rank, columns, c);
                                halfSpaceConstraints(ref c);
                                for (int j = 0; j < (N_DIMENSIONS + 1); j++)
                                {
                                    constraints[c_base + c_counter] = c[j];
                                    c_counter++;
                                }
                            }

                        }
                        else
                        {
                            float minC;
                            float maxC;
                            int coord = 0;

                            for (int i = 0; i < N_DIMENSIONS; i++)
                                if (proj_base[i] != 0)
                                    coord = i;

                            minC = float.MaxValue;
                            maxC = float.MinValue;
                            for (int p = 0; p < N_DIMENSIONS; p++)
                            {
                                minC = Math.Min(minC, points[coord, p]);
                                maxC = Math.Max(maxC, points[coord, p]);
                            }

                            float[] consMin = new float[N_DIMENSIONS + 1];
                            float[] consMax = new float[N_DIMENSIONS + 1];

                            for (int i = 0; i < N_DIMENSIONS; i++)
                            {
                                if (i == coord)
                                    consMin[i] = -1;
                                else
                                    consMin[i] = 0;
                            }
                            consMin[N_DIMENSIONS] = minC;

                            for (int i = 0; i < N_DIMENSIONS; i++)
                            {
                                if (i == coord)
                                    consMax[i] = 1;
                                else
                                    consMax[i] = 0;
                            }
                            consMax[N_DIMENSIONS] = -maxC;

                            halfSpaceConstraints(ref consMin);
                            halfSpaceConstraints(ref consMax);

                            for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                            {
                                constraints[c_base + c_counter] = consMin[i];
                                c_counter++;
                            }

                            for (int i = 0; i < (N_DIMENSIONS + 1); i++)
                            {
                                constraints[c_base + c_counter] = consMax[i];
                                c_counter++;
                            }
                        }
                    }
                }
            }
            ); // End of Parallel.For
        }

        public string getConstraintsStr()
        {
            StringWriter sw = new StringWriter(CultureInfo.InvariantCulture.NumberFormat);
            
            for (int splx = 0; splx < numSimplices; splx++)
            {
                int cBase = (N_DIMENSIONS + 1) * C_PER_SIMPLEX * splx;
                for (int constraint = 0; constraint < C_PER_SIMPLEX; constraint++)
                {
                    for (int coef = 0; coef < N_DIMENSIONS + 1; coef++)
                    {
                        sw.Write("{0} ", constraints[cBase + constraint * (N_DIMENSIONS + 1) + coef]);
                    }
                    sw.Write("\n");
                }
                sw.Write("\n");
            }

            return sw.ToString();
        }
    }
}
