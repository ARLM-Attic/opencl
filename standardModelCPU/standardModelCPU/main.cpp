#include <iostream>

#include "EuclideanObject.h"
#include "LinearAlgebra.h"

using namespace std;

int main(int argc, char **argv) {
	Matrix m;

	for (int i = 0; i < N+1; i++) {
		for (int j = 0; j < K+1; j++) {
			m[i][j] = 0;
		}
	}

	m[0][0] = 0.9298;
	m[0][1] = 0.9143;
	m[0][2] = -0.7162;
	m[1][0] = -0.6848;
	m[1][1] = -0.0292;
	m[1][2] = -0.1565;
	m[2][0] = 0.9412;
	m[2][1] = 0.6006;
	m[2][2] = 0.8315;
	m[3][0] = 1;
	m[3][1] = 1;
	m[3][2] = 1;

	printMatrix(m);

	vector<constraint*> v = stSimplex(m);

	for (int i = 0; i < v.size(); i++) {
		printConstraint(v[i]);
		cout << endl;
	}

	system("pause");
}