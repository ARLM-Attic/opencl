#include <ctime>
#include <iostream>
#include <fstream>
#include <ctime>

#include <oclUtils.h>
#include "OpenCL.h"

//#include "XForm.h"
//#include "GLCamera.h"

#include "nchoosek.h"
#include "stSimplex.h"
#include "tester.h"

using namespace ocl;
using namespace std;

#define T 1.0e-2f

/* Rasterizador ******************************/
#define GRID_SIZE_X 180
#define GRID_SIZE_Y 180
#define GRID_SIZE_Z 180
#define VOLUME_SIZE ((GRID_SIZE_X) * (GRID_SIZE_Y) * (GRID_SIZE_Z))

int    SIMPLICES;
int    s_size;
float* simplices;
int    c_size;
float* c_check;
bool   volume[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
/*********************************************/
/* CL ****************************************/
OpenCL   cl;
Buffer*  pbo_cl;
Program* progStRender;
Launcher stRender;
Launcher stFillVolume;
Buffer*  constraints_d;
Buffer*  simplices_d;
Buffer*	 volume_d;
/*********************************************/
/* GL ****************************************/
vector<float> points;
vector<float> axis;

unsigned int width = 512, height = 512;

unsigned int iGLUTWindowHandle;

void render();

void initDrawArray();
void InitGL(int argc, const char** argv);
void DisplayGL();
void KeyboardGL(unsigned char key, int x, int y);
void Reshape(int w, int h);
void motion(int x, int y);
void mouseFunction(int button, int state, int x, int y);
void Idle(void);
/*********************************************/
/* Camera ************************************/
/*xform thexform;
xform global_xf;
GLCamera camera;
string xffilename;
static unsigned buttonstate = 0;
*/
/*********************************************/

float* loadDataset(const char* path, int& num_simplices);

bool equal(float value1, float value2) {
	return (fabs(value1-value2) < fabs(T));
}

void readHeightMap(const char* filename) {
	simplices = loadDataset(filename, SIMPLICES);
	c_size = (N+1)*CONSTRAINTS;
	s_size = (K+1)*SIMPLICES;
}

void readTriangles(const char* filename) {
	FILE* splxFile;
	if(filename) splxFile = fopen(filename, "r");

	if(!filename || splxFile == NULL) {
		cout << "Using random simplices..." << endl;
		SIMPLICES = 1; //_SIMPLICES;

		c_size = (N+1)*CONSTRAINTS;
		// each point in a column
		s_size = (N+1)*(K+1)*SIMPLICES;
		simplices = new float[s_size];

		//Fill the matrix randomly
		srand(time(0));
		for (int i = 0; i < s_size; i++) {
			simplices[i] = (float)(rand()%1024);
			if (i%((N+1)*(K+1))>=(N*(K+1)))
				simplices[i] = 1;
		}
	}
	else {
		cout << "Using " << filename << " simplices..." << endl;
		fscanf(splxFile, "%d", &SIMPLICES);
		c_size = (N+1)*CONSTRAINTS;
		s_size = (N+1)*(K+1)*SIMPLICES;
		simplices = new float[s_size];
		//Read the file
		//Simplex s, point p, coordinate dim
		for (int s=0; s<SIMPLICES; s++) {
			for(int dim=0; dim<N; dim++) {			
				for(int p=0; p<K+1; p++) {
					fscanf(splxFile, "%f ", &simplices[dim*(SIMPLICES)*(K+1) + s*(K+1) + p]);
					//homogenous coordinates
					simplices[N*(SIMPLICES)*(K+1) + s*(K+1) + p] = 1;
				}
			}
		}

		fclose(splxFile);
	}
}

void clearVolume() {
	for(int z=0; z<GRID_SIZE_Z; z++)
		for(int y=0; y<GRID_SIZE_Y; y++)
			for(int x=0; x<GRID_SIZE_X; x++)
				volume[x][y][z] = false;
}

double getMiliseconds(const int time) {
	return 1000*time/(float)CLOCKS_PER_SEC;
}

void fillVolume() {
	for(int idx = 0; idx < SIMPLICES; idx++) {
		const int c_base = (N+1)*C_PER_SIMPLEX*idx;

		int c_counter = C_PER_SIMPLEX*(N+1);

		SMatrix points;
		// Load matrix into registers
		for (int i = 0; i < (K+1)*(N+1); i++) {
			const int ln = i/(K+1);
			const int cl = i%(K+1);
			points[ln][cl] = simplices[idx*(K+1) + ln*SIMPLICES*(K+1) + cl];
		}

		float minCoord[] = {9999, 9999, 9999};
		float maxCoord[] = {-9999, -9999, -9999};
		for(int coord=0; coord<N; coord++)
		{
			for(int p=0; p<N; p++)
			{
				minCoord[coord] = min(minCoord[coord], points[coord][p]);
				maxCoord[coord] = max(maxCoord[coord], points[coord][p]);
			}
			// get the floor of the min value and the ceil of the max
			minCoord[coord] = (int) minCoord[coord];
			maxCoord[coord] = (int) (maxCoord[coord] + 1);
		}

		for(int vX=(int)minCoord[0]; vX<=(int)maxCoord[0]; vX++)
			for(int vY=(int)minCoord[1]; vY<=(int)maxCoord[1]; vY++)
				for(int vZ=(int)minCoord[2]; vZ<=(int)maxCoord[2]; vZ++)
				{
					float discreteP[] = {vX, vY, vZ};

					bool raster = true;
					for(int i=0; i < c_counter/(N+1); i++)
					{
						float soma = 0;
						for(int coord=0; coord<N; coord++) {
							soma += discreteP[coord] * c_check[c_base + i*(N+1) + coord];
						}

						//if(vX==0 && vY==0 && vZ==0) cout << "fv: " << -c_check[c_base + i*(N+1) + N] << endl;

						if(!(soma <= -c_check[c_base + i*(N+1) + N])) {
							raster = false;
							break;
						}
					}
					//if(vX==0 && vY==0 && vZ==0) cout << "====" << endl;

					if(raster && vX<GRID_SIZE_X && vY<GRID_SIZE_Y && vZ<GRID_SIZE_Z && vX>=0 && vY>=0 && vZ>=0) {
						//if(vX==0 && vY==0 && vZ==0) cout << "abc " << endl;
						volume[vX][vY][vZ] = true;
					}
				}
	}
}

void writeVolume(const char* filename) {
	FILE* volOut = NULL;
	if(filename) volOut = fopen(filename, "w");

	cout << "Writing volume to file... ";
	if(volOut) {
		for(int y=0; y<GRID_SIZE_Y; y++) {
			for(int z=0; z<GRID_SIZE_Z; z++) {
				for(int x=0; x<GRID_SIZE_X; x++)
					fprintf(volOut, "%d ", volume[x][y][z]);
				fprintf(volOut, "\n");
			}
			fprintf(volOut, "\n");
		}
		fclose(volOut);
		cout << "done." << endl;
	}
	else cout << "error." << endl;
}

int main(int argc, const char **argv) {
	//K = number of points
	//N = dimensionality

	// Projections matrix
	cout << "Allocating projections matrix..." << endl;
	int nckRows;
	const int _m_ = (N < K+1)?(N):(K+1);
	int* nckv = nchoosekVector(N, _m_, &nckRows);
	const int nck_size = (N+2)*nckRows*sizeof(int);
	Buffer* nck_d = cl.createBuffer(nck_size, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, nckv);
	/*************************************************************************************************************/
	float* constraints;
	
	//bool fromHeightMap = false;
	bool fromHeightMap = true;
	if(fromHeightMap)
		readHeightMap("../datasets/d1.txt");
		//readHeightMap("../datasets/d_small.txt");
	else
		readTriangles("simplices_in.txt");

	constraints = new float[c_size];
	// Initialize the constraints so that evaluation is always true (no constraint)
	for (int c = 0; c < CONSTRAINTS*(N+1); c++) {
		constraints[c] = 0;
		/*
		if ((c+1)%(N+2)!=0)
			constraints[c] = 0;
		else
			constraints[c] = -1.0f;
		//*/
	}

	constraints_d =
		cl.createBuffer(c_size*sizeof(float), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, constraints);
	simplices_d = cl.createBuffer(s_size*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices);
	/*************************************************************************************************************/

	cout << "Building program..." << endl;
	vector<string> files;
	files.push_back("stSimplex.h");
	files.push_back("stSimplex2.cl");
	Program *program = cl.createProgram(files);
	program->build(true);

	const size_t global_ws = shrRoundUp(LOCAL_WORKSIZE, SIMPLICES);

	cout << "Enqueueing arguments..." << endl;
	Launcher stSimplex = program->createLauncher("stSimplex").global(global_ws).local(LOCAL_WORKSIZE);

	clearVolume();

	volume_d =
		cl.createBuffer(VOLUME_SIZE*sizeof(bool), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, volume);
	
	stSimplex.arg(simplices_d->getMem()).arg(constraints_d->getMem()).arg(nck_d->getMem()).arg(nckRows).arg(SIMPLICES);
	stSimplex.arg(volume_d->getMem()).arg(GRID_SIZE_X).arg(GRID_SIZE_Y).arg(GRID_SIZE_Z);

	stSimplex.run(); cl.finish();
	cout << "Launching kernel..." << endl;
	clock_t gpu_b = clock();
	stSimplex.run();
	cl.finish();
	clock_t gpu_a = clock();

	int gpu_d = gpu_a - gpu_b;
	cout << "Kernel execution time (ms): " << getMiliseconds(gpu_d) << endl;
	
	cout << "Reading back... ";
	c_check = new float[c_size];
	constraints_d->read(c_check, c_size*sizeof(float));
	cout << "done." << endl;

	//* Print GPU Result
	cout << "Writing inequations to file... ";
	FILE* out = fopen("ineq.txt", "w");
	//FILE* out = stdin;
	if(out) {
		for (int s=0; s<SIMPLICES; s++) {
			for(int c=0; c<C_PER_SIMPLEX; c++) {
				for(int dim=0; dim<N+1; dim++) {
					fprintf(out, "%f ", c_check[s*(C_PER_SIMPLEX)*(N+1) + c*(N+1) + dim]);
					//fprintf(out, "%f ", constraints[s*(C_PER_SIMPLEX)*(N+1) + c*(N+1) + dim]);
				}
				fprintf(out, "\n");
			}
			fprintf(out, "\n");
		}
		fclose(out);
		cout << "done." << endl;
	}
	else cout << "error." << endl;
	//*/

	cout << "Filling volume... ";
	clock_t fv_b = clock();
	fillVolume();
	clock_t fv_a = clock();
	cout << "done." << endl;

	const int fv_d = fv_a - fv_b;
	cout << "Volume fill time (ms): " << getMiliseconds(fv_d) << endl;

	writeVolume("volume_misto.txt");

	cout << "Performing computation on CPU" << endl;
	
	//clearVolume();
	clock_t cpu_b = clock();
	stSimplexCPU(simplices, constraints, nckv, nckRows, SIMPLICES, (bool*)volume, GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z);
	clock_t cpu_a = clock();
	//writeVolume("volume.txt");

	const int cpu_d = cpu_a - cpu_b;
	cout << "CPU execution time (ms): " << getMiliseconds(cpu_d) << endl;

	gpu_d += fv_d; // time to calculate de constraints + time to fill the volume
	cout << "GPU is " << cpu_d/(float)gpu_d << " times faster than CPU" << endl;
	
	cout << "Checking..." << endl;
	int dif = 0;
	int equ = 0;
	for (int i = 0; i < CONSTRAINTS; i++) {
		if (!equal(constraints[i], c_check[i])) {
			dif++;
			//cout << i << ": " << constraints[i] << " != " << c_check[i] << endl;
		}
		else
			equ++;
	}

	cout << dif << " different values out of " << CONSTRAINTS << endl;
	cout << 100*float(dif)/(float)CONSTRAINTS << "% wrong" << endl;

	InitGL(argc, argv);
	initDrawArray();
	glutMainLoop();
}

float* loadDataset(const char* path, int& num_simplices) {
	std::ifstream df;
	df.open(path);
	int sx = 0, sy = 0;
	df >> sx >> sy;

	float** heights = new float*[sx];
	for (int i = 0; i < sx; i++) {
		heights[i] = new float[sy];
		for (int j = 0; j < sy; j++)
			heights[i][j] = 0;
	}
	for (int row = 0; row < sx; row++) {
		for (int col = 0; col < sy; col++) {
			df >> heights[row][col];
		}
	}
	df.close();

	/* write subset
	cout << "Writing subset..." << endl;
	int ssX=30;
	int ssY=30;
	FILE* fo = fopen("../datasets/d_small.txt", "w");
	fprintf(fo, "%d %d\n", ssX, ssY);
	for(int i=0; i<ssX; i++) {
		for(int j=0; j<ssY; j++)
			fprintf(fo, "%f ", heights[i][j]);
		fprintf(fo, "\n");
	}
	fclose(fo);
	//*/

	num_simplices = sx*sy*2 ;
	const int size = num_simplices  * (K+1) * (N+1); // x1, x2, x3, 1
	float* simplices = new float[size];

	for (int i=0; i<size; i++)
		simplices[i] = 0;

	const int ns = num_simplices * (K+1);
	int idx = 0;
	for (int y = 0; y < sy - 1; y++) {
		for (int x = 0; x < sx - 1; x++) {
			// Even - 1st triangle
			simplices[idx] = x; // 1st coord, 1st point
			simplices[idx + ns] = y; // 2nd coord, 1st point
			simplices[idx + 2*ns] = heights[x][y]; // 3rd coord, 1st point
			simplices[idx + 3*ns] = 1; //homogenous coord, 1st point
			idx++;

			simplices[idx] = x+1; // 1st coord, 2nd point
			simplices[idx + ns] = y; // 2nd coord, 2nd point
			simplices[idx + 2*ns] = heights[x+1][y]; // 3rd coord, 2nd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 2nd point
			idx++;

			simplices[idx] = x+1; // 1st coord, 3rd point
			simplices[idx + ns] = y+1; // 2nd coord, 3rd point
			simplices[idx + 2*ns] = heights[x+1][y+1]; // 3rd coord, 3rd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 3rd point
			idx++;

			// Odd - 2nd triangle
			simplices[idx] = x; // 1st coord, 1st point
			simplices[idx + ns] = y; // 2nd coord, 1st point
			simplices[idx + 2*ns] = heights[x][y]; // 3rd coord, 1st point
			simplices[idx + 3*ns] = 1; //homogenous coord, 1st point
			idx++;

			simplices[idx] = x+1; // 1st coord, 2nd point
			simplices[idx + ns] = y+1; // 2nd coord, 2nd point
			simplices[idx + 2*ns] = heights[x+1][y+1]; // 3rd coord, 2nd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 2nd point
			idx++;

			simplices[idx] = x; // 1st coord, 3rd point
			simplices[idx + ns] = y+1; // 2nd coord, 3rd point
			simplices[idx + 2*ns] = heights[x][y+1]; // 3rd coord, 3rd point
			simplices[idx + 3*ns] = 1; //homogenous coord, 3rd point
			idx++;
		}
	}

	for (int i = 0; i < sx; i++)
		delete[] heights[i];
	delete[] heights;

	return simplices;
}

void InitGL(int argc, const char **argv)
{
	// initialize GLUT 
	glutInit(&argc, (char **)argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - width/2, 
		glutGet(GLUT_SCREEN_HEIGHT)/2 - height/2);
	glutInitWindowSize(width, height);
	iGLUTWindowHandle = glutCreateWindow("OpenCL volume rendering");

	// register glut callbacks
	glutDisplayFunc(DisplayGL);
	glutKeyboardFunc(KeyboardGL);
	glutIdleFunc(Idle);
	glutMouseFunc(mouseFunction);
	glutMotionFunc(motion);
	glutReshapeFunc(Reshape);

	glewInit();

	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
}

void initDrawArray()
{
	double cubeSize = 2.0/((double)GRID_SIZE_X);
	for(int y=0; y<GRID_SIZE_Y; y++)
		for(int z=0; z<GRID_SIZE_Z; z++)
			for(int x=0; x<GRID_SIZE_X; x++)
				if(volume[x][y][z])
				{
					points.push_back((x+0.5)*cubeSize);
					points.push_back((z+0.5)*cubeSize);
					points.push_back((y+0.5)*cubeSize);
				}
}


// render image using OpenCL
//*****************************************************************************
void render()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
		glPushMatrix();
			glColor3f(0, 1, 0);
			glBegin(GL_LINES);
				glVertex3f(0,0,0);
				glVertex3f(10,0,0);
				glVertex3f(0,0,0);
				glVertex3f(0,10,0);
				glVertex3f(0,0,0);
				glVertex3f(0,0,10);
			glEnd();
		glPopMatrix();

		glPushMatrix();
			glColor3f(1, 0, 0);

			glEnableClientState(GL_VERTEX_ARRAY);
			glVertexPointer(3, GL_FLOAT, 0, &points[0]);
			glDrawArrays(GL_POINTS, 0, points.size()/3);
			glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	glPopMatrix();
}

// Display callback for GLUT main loop
//*****************************************************************************
void DisplayGL()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0); 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	static float rot=0;
	glRotatef(rot, 0, 1, 0);
	rot += 0.1;

//	camera.setupGL(global_xf * point(0,0,0), 1.7);
    //glPushMatrix();
//    glMultMatrixd(global_xf);

	glTranslatef(-0.9, -0.9, -0.9);
	glRotatef(5, 1, 0, 1);

	// process 
	render();

	// flip backbuffer to screen
	glutSwapBuffers();
	glutPostRedisplay();
}

// Keyboard event handler callback
//*****************************************************************************
void KeyboardGL(unsigned char key, int /*x*/, int /*y*/)
{
    glutPostRedisplay();
}

void mouseFunction(int button, int state, int x, int y)
{

	glutPostRedisplay();
}

void motion(int x, int y)
{

	glutPostRedisplay();
}

void Reshape(int x, int y)
{
	width = x; height = y;

	glViewport(0, 0, x, y);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0.0, 1.0, 0.0, 1.0, 0.0, 1.0); 
}

// GL Idle time callback
//*****************************************************************************
void Idle()
{

    glutPostRedisplay();
}
