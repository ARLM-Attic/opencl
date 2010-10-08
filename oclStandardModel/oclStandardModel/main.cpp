#include <ctime>
#include <iostream>
#include <fstream>
#include <ctime>

#include <oclUtils.h>
#include "OpenCL.h"

#include "XForm.h"
#include "GLCamera.h"

#include "nchoosek.h"
#include "stSimplex.h"
#include "tester.h"

using namespace ocl;
using namespace std;

#define T 1.0e-2f

/* CL ****************************************/
OpenCL cl;
Buffer* pbo_cl;
Program *progStRender;
Launcher stRender;
Launcher stFillVolume;
Buffer* constraints_d;
Buffer* simplices_d;

#define GRID_SIZE_X 180
#define GRID_SIZE_Y 180
#define GRID_SIZE_Z 180
int	volume[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
Buffer*	volume_d;
/*********************************************/
/* GL *****************************************/
cl_int ciErrNum;

GLuint pbo = 0;                 // OpenGL pixel buffer object
vector<float> points;

unsigned int width = 512, height = 512;
size_t gridSize[2] = {width, height};

#define LOCAL_SIZE_X 16
#define LOCAL_SIZE_Y 16

unsigned int iGLUTWindowHandle;
float viewRotation[3];
float viewTranslation[3] = {0.0, 0.0, -4.0f};
float invViewMatrix[12];
void DisplayGL();


void initPixelBuffer();
void render();
void initCLVolume(unsigned char *h_volume);

int ox, oy;                         // mouse location vars
int buttonState = 0;       

void initDrawArray();
void InitGL(int argc, const char** argv);
void DisplayGL();
void KeyboardGL(unsigned char key, int x, int y);
void Reshape(int w, int h);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void Idle(void);

void Cleanup(int iExitCode);
void (*pCleanup)(int) = &Cleanup;

bool imgSupport;
/*********************************************/
xform thexform;
xform global_xf;
GLCamera camera;
string xffilename;
static unsigned buttonstate = 0;
/*********************************************/

float* loadDataset(const char* path, int& num_simplices);

bool equal(float value1, float value2) {
	return (fabs(value1-value2) < fabs(T));
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

	cout << "globalsize: " << cl.getGlobalMemSize() << endl;
	cout << "localsize: " << cl.getLocalMemSize() << endl;
	cout << "maxMemAllocSize: " << cl.getMaxMemAllocSize() << endl;

	/*************************************************************************************************************/
	int SIMPLICES;
	int s_size;
	float* simplices;
	int c_size;
	float* constraints;
	
	//* Read HeightMap
	simplices = loadDataset("../datasets/d1.txt", SIMPLICES);
	c_size = (N+1)*CONSTRAINTS;
	s_size = (K+1)*SIMPLICES;
	//*/

	/* Read from generic file
	FILE* splxFile = fopen("simplices_in.txt", "r");
	if(splxFile == NULL) {
		SIMPLICES = 1;//_SIMPLICES;

		// Constraints matrix
		cout << "Allocating constraints matrix..." << endl;
		c_size = (N+1)*CONSTRAINTS;#include "XForm.h"
#include "GLCamera.h"

		// Simplices matrix
		cout << "Allocating simplices matrix..." << endl;
		// each point in a column
		s_size = (N+1)*(K+1)*SIMPLICES;
		simplices = new float[s_size];

		cout << "Filling the matrix - random" << endl;
		//Fill the matrix randomly
		srand(time(0));
		for (int i = 0; i < s_size; i++) {
			int d = rand();
			simplices[i] = (float)(rand()%1024);
			if (i%((N+1)*(K+1))>=(N*(K+1)))
				simplices[i] = 1;
		}
	}
	else {
		fscanf(splxFile, "%d", &SIMPLICES);

		// Constraints matrix
		cout << "Allocating constraints matrix..." << endl;
		c_size = (N+1)*CONSTRAINTS;

		// Simplices matrix
		cout << "Allocating simplices matrix..." << endl;#include "XForm.h"
#include "GLCamera.h"
		s_size = (N+1)*(K+1)*SIMPLICES;
		simplices = new float[s_size];

		cout << "Filling the matrix - reading from file" << endl;
		//Read the file
		//Simplex s, point p, coordinate dim
		for (int s=0; s<SIMPLICES; s++) {
			for(int dim=0; dim<N; dim++) {			
				for(int p=0; p<K+1; p++) {
					//fscanf(splxFile, "%f ", &simplices[s*(N+1)*(K+1) + dim*(K+1) + p]);
					fscanf(splxFile, "%f ", &simplices[dim*(SIMPLICES)*(K+1) + s*(K+1) + p]);
					//homogenous coordinates
					simplices[N*(SIMPLICES)*(K+1) + s*(K+1) + p] = 1;
					//simplices[s*(N+1)*(K+1) + N*(K+1) + p] = 1;
				}
			}
		}

		fclose(splxFile);
	}
	//*/

	constraints = new float[c_size];
	// Initialize the constraints so that evaluation is always true (no constraint)
	for (int c = 0; c < CONSTRAINTS*(N+1); c++) {
		if ((c+1)%(N+2)!=0)
			constraints[c] = 0;
		else
			constraints[c] = -1.0f;
	}

	cout << "constraints size: " << c_size*sizeof(float) << endl;
	cout << "simplices size: " << s_size*sizeof(float) << endl;
	cout << "volume size: " << 180*180*180 << endl;
	constraints_d =
		cl.createBuffer(c_size*sizeof(float), CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR, constraints);
	simplices_d = cl.createBuffer(s_size*sizeof(float), CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, simplices);
	/*************************************************************************************************************/

	cout << "Building program..." << endl;
	vector<string> files;
	files.push_back("stSimplex.h");
	files.push_back("stSimplex2.cl");
	Program *program = cl.createProgram(files);
	program->build("-cl-nv-verbose");

	const size_t global_ws = shrRoundUp(LOCAL_WORKSIZE, SIMPLICES);

	cout << "Enqueueing arguments..." << endl;
	Launcher stSimplex = program->createLauncher("stSimplex").global(global_ws).local(LOCAL_WORKSIZE);
	
	vector<string> files2;
	files2.push_back("stSimplex.h");
	files2.push_back("render.cl");
	progStRender = cl.createProgram(files2);
	progStRender->build();
	stRender = progStRender->createLauncher("d_render");
	stFillVolume = progStRender->createLauncher("fill_volume");

	for(int z=0; z<GRID_SIZE_Z; z++)
		for(int y=0; y<GRID_SIZE_Y; y++)
			for(int x=0; x<GRID_SIZE_X; x++)
				volume[x][y][z] = 0;

	volume_d =
		cl.createBuffer(GRID_SIZE_X*GRID_SIZE_Y*GRID_SIZE_Z*sizeof(int), CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, volume);
	
	stSimplex.arg(simplices_d->getMem()).arg(constraints_d->getMem()).arg(nck_d->getMem()).arg(nckRows).arg(SIMPLICES);
	stSimplex.arg(volume_d->getMem()).arg(GRID_SIZE_X).arg(GRID_SIZE_Y).arg(GRID_SIZE_Z);
	cout << "Launching kernel..." << endl;

	clock_t gpu_b = clock();
	stSimplex.run();
	cl.finish();
	clock_t gpu_a = clock();

	const int gpu_d = gpu_a - gpu_b;
	cout << "Kernel execution time (ms): " << 1000*gpu_d/(float)CLOCKS_PER_SEC << endl;

	
	/*ofstream gpu, cpu;
	gpu.open("gpu.txt");
	cpu.open("cpu.txt");
	*/
	
	// PERFORMING TEST
	cout << "Reading back..." << endl;
	float* c_check = new float[c_size];
	
	// (N+1) * constraints
	// CONSTRAINTS = (SIMPLICES*C_PER_SIMPLEX)
	constraints_d->read(c_check, c_size*sizeof(float));
	FILE* out = fopen("hm_out.txt", "w");
	//FILE* out = stdin;
	//* Print GPU Result
	for (int s=0; s<SIMPLICES; s++) {
		for(int c=0; c<C_PER_SIMPLEX; c++) {
			for(int dim=0; dim<N+1; dim++) {
				fprintf(out, "%f ", c_check[s*(C_PER_SIMPLEX)*(N+1) + c*(N+1) + dim]);
			}
			fprintf(out, "\n");
		}
		fprintf(out, "\n");
	}
	fclose(out);
	//*/

	volume_d->read(volume, GRID_SIZE_X*GRID_SIZE_Y*GRID_SIZE_Z*sizeof(int));

	FILE* volOut = fopen("volume_gpu.txt", "w");
	for(int y=0; y<GRID_SIZE_Y; y++) {
		for(int z=0; z<GRID_SIZE_Z; z++) {
			for(int x=0; x<GRID_SIZE_X; x++)
				fprintf(volOut, "%d ", volume[x][y][z]);
			fprintf(volOut, "\n");
		}
		fprintf(volOut, "\n");
	}
	fclose(volOut);


	clock_t fv_b = clock();
	for(int idx = 0; idx < SIMPLICES; idx++) {
		const int c_base = (N+1)*C_PER_SIMPLEX*idx;

		int c_counter = 0;

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

						if(!(soma <= -c_check[c_base + i*(N+1) + N])) {
							raster = false;
							break;
						}
					}

					if(raster && vX<GRID_SIZE_X && vY<GRID_SIZE_Y && vZ<GRID_SIZE_Z && vX>=0 && vY>=0 && vZ>=0) {
						volume[vX][vY][vZ] = 1;
					}
				}
	}
	clock_t fv_a = clock();

	const int fv_d = fv_a - fv_b;
	cout << "Volume fill time (ms): " << 1000*fv_d/(float)CLOCKS_PER_SEC << endl;


	volOut = fopen("volume_misto.txt", "w");
	for(int y=0; y<GRID_SIZE_Y; y++) {
		for(int z=0; z<GRID_SIZE_Z; z++) {
			for(int x=0; x<GRID_SIZE_X; x++)
				fprintf(volOut, "%d ", volume[x][y][z]);
			fprintf(volOut, "\n");
		}
		fprintf(volOut, "\n");
	}
	fclose(volOut);


	cout << "Performing computation on CPU" << endl;
							
	clock_t cpu_b = clock();
	stSimplexCPU(simplices, constraints, nckv, nckRows, SIMPLICES, (int*)volume, GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z);
	clock_t cpu_a = clock();


	volOut = fopen("volume.txt", "w");
	for(int y=0; y<GRID_SIZE_Y; y++) {
		for(int z=0; z<GRID_SIZE_Z; z++) {
			for(int x=0; x<GRID_SIZE_X; x++)
				fprintf(volOut, "%d ", volume[x][y][z]);
			fprintf(volOut, "\n");
		}
		fprintf(volOut, "\n");
	}
	fclose(volOut);

	/* Print CPU Result
	for (int s=0; s<SIMPLICES; s++) {
		for(int c=0; c<C_PER_SIMPLEX; c++) {
			for(int dim=0; dim<N+1; dim++) {
				fprintf(out, "%f ", constraints[s*(C_PER_SIMPLEX)*(N+1) + c*(N+1) + dim]);
			}
			fprintf(out, "\n");
		}
		fprintf(out, "\n");
	}
	//*/

	const int cpu_d = cpu_a - cpu_b;
	cout << "CPU execution time (ms): " << 1000*cpu_d/(float)CLOCKS_PER_SEC << endl;

	cout << "GPU is " << cpu_d/(float)gpu_d << " times faster than CPU" << endl;
	
	cout << "Checking..." << endl;
	int dif = 0;
	int equ = 0;
	for (int i = 0; i < CONSTRAINTS; i++) {
		if (!equal(constraints[i], c_check[i]))
			dif++;
		else
			equ++;
	}

	cout << dif << " different values out of " << CONSTRAINTS << endl;
	cout << 100*float(dif)/(float)CONSTRAINTS << "% wrong" << endl;

	cout << "Press enter to continue..." << endl;
	getchar();

	/*
	stFillVolume.arg(volume_d->getMem()).arg(constraints_d->getMem()).arg(GRID_SIZE_X);
	stFillVolume.arg(GRID_SIZE_Y).arg(GRID_SIZE_Z).arg(CONSTRAINTS);

	const size_t localFV[3] = {4, 4, 4};
	//const size_t globalFV[3] = {shrRoundUp(localFV[0], GRID_SIZE_X), 
	//						shrRoundUp(localFV[1], GRID_SIZE_Y), shrRoundUp(localFV[2], GRID_SIZE_Z)};
	const size_t globalFV[3] = {32, 32,32};
	stFillVolume.local(localFV[0], localFV[1], localFV[2]).global(globalFV[0], globalFV[1], globalFV[2]);
	stFillVolume.run();
	*/

	InitGL(argc, argv);
	initDrawArray();
	glutMainLoop();
}

float* loadDataset(const char* path, int& num_simplices) {
	std::ifstream df;
	df.open(path);
	int sx = 0, sy = 0;
	df >> sx >> sy;
	// loads buffer matrix
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

	//num_simplices = (sx-1)*(sy-1)*2;
	num_simplices = sx*sy*2 ;
	const int size = num_simplices  * (K+1) * (N+1);	// x1, x2, x3, 1
	float* simplices = new float[size];

	for (int i=0; i<size; i++)
		simplices[i] = 0;

	const int ns = num_simplices * (K+1);
	int idx = 0;
	for (int y = 0; y < sy - 1; y++) {
		for (int x = 0; x < sx - 1; x++) {
		// Versao Bruno:
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

		/* Versao Leonardo:
			// even
			simplices[idx] = heights[x][y];	// 1st
			simplices[idx + num_simplices] = heights[x+1][y]; // 2nd
			simplices[idx + 2*num_simplices] = heights[x+1][y+1]; // 3rd
			idx++;
			// odd
			simplices[idx] = heights[x][y];
			simplices[idx + num_simplices] = heights[x+1][y+1];
			simplices[idx + 2 * num_simplices] = heights[x][y+1];
			idx++;
		*/
		}
	}

	// cleanup
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
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(Reshape);

	glewInit();
}

void initDrawArray()
{
	double cubeSize = 2.0/((double)GRID_SIZE_X);
	for(int y=0; y<GRID_SIZE_Y; y++)
			for(int z=0; z<GRID_SIZE_Z; z++)
				for(int x=0; x<GRID_SIZE_X; x++)
					if(volume[x][y][z])
					{
						/*
						float p[2][2][2][3];
						
						p[0][0][0][0] = x*cubeSize;
						p[0][0][0][1] = y*cubeSize;
						p[0][0][0][2] = z*cubeSize;

						p[1][0][0][0] = (x+1)*cubeSize;
						p[1][0][0][1] = y*cubeSize;
						p[1][0][0][2] = z*cubeSize;

						p[1][1][0][0] = (x+1)*cubeSize;
						p[1][1][0][1] = (y+1)*cubeSize;
						p[1][1][0][2] = z*cubeSize;

						p[0][1][0][0] = x*cubeSize;
						p[0][1][0][1] = (y+1)*cubeSize;
						p[0][1][0][2] = z*cubeSize;

						p[0][0][1][0] = x*cubeSize;
						p[0][0][1][1] = y*cubeSize;
						p[0][0][1][2] = (z+1)*cubeSize;

						p[1][0][1][0] = (x+1)*cubeSize;
						p[1][0][1][1] = y*cubeSize;
						p[1][0][1][2] = (z+1)*cubeSize;

						p[1][1][1][0] = (x+1)*cubeSize;
						p[1][1][1][1] = (y+1)*cubeSize;
						p[1][1][1][2] = (z+1)*cubeSize;

						p[0][1][1][0] = x*cubeSize;
						p[0][1][1][1] = (y+1)*cubeSize;
						p[0][1][1][2] = (z+1)*cubeSize;

						// front face
						points.push_back(p[0][0][0][0]);points.push_back(p[0][0][0][1]);points.push_back(p[0][0][0][2]);
						points.push_back(p[1][0][0][0]);points.push_back(p[1][0][0][1]);points.push_back(p[1][0][0][2]);
						points.push_back(p[1][1][0][0]);points.push_back(p[1][1][0][1]);points.push_back(p[1][1][0][2]);
						points.push_back(p[0][1][0][0]);points.push_back(p[0][1][0][1]);points.push_back(p[0][1][0][2]);

						// back face
						points.push_back(p[0][0][1][0]);points.push_back(p[0][0][1][1]);points.push_back(p[0][0][1][2]);
						points.push_back(p[0][1][1][0]);points.push_back(p[0][1][1][1]);points.push_back(p[0][1][1][2]);
						points.push_back(p[1][1][1][0]);points.push_back(p[1][1][1][1]);points.push_back(p[1][1][1][2]);
						points.push_back(p[1][0][1][0]);points.push_back(p[1][0][1][1]);points.push_back(p[1][0][1][2]);

						// left face
						points.push_back(p[0][0][0][0]);points.push_back(p[0][0][0][1]);points.push_back(p[0][0][0][2]);
						points.push_back(p[0][1][0][0]);points.push_back(p[0][1][0][1]);points.push_back(p[0][1][0][2]);
						points.push_back(p[0][1][1][0]);points.push_back(p[0][1][1][1]);points.push_back(p[0][1][1][2]);
						points.push_back(p[0][0][1][0]);points.push_back(p[0][0][1][1]);points.push_back(p[0][0][1][2]);

						// right face
						points.push_back(p[1][0][0][0]);points.push_back(p[1][0][0][1]);points.push_back(p[1][0][0][2]);
						points.push_back(p[1][0][1][0]);points.push_back(p[1][0][1][1]);points.push_back(p[1][0][1][2]);
						points.push_back(p[1][1][1][0]);points.push_back(p[1][1][1][1]);points.push_back(p[1][1][1][2]);
						points.push_back(p[1][1][0][0]);points.push_back(p[1][1][0][1]);points.push_back(p[1][1][0][2]);

						// bottom face
						points.push_back(p[0][0][0][0]);points.push_back(p[0][0][0][1]);points.push_back(p[0][0][0][2]);
						points.push_back(p[1][0][0][0]);points.push_back(p[1][0][0][1]);points.push_back(p[1][0][0][2]);
						points.push_back(p[1][0][1][0]);points.push_back(p[1][0][1][1]);points.push_back(p[1][0][1][2]);
						points.push_back(p[0][0][1][0]);points.push_back(p[0][0][1][1]);points.push_back(p[0][0][1][2]);

						// top face
						points.push_back(p[0][1][0][0]);points.push_back(p[0][1][0][1]);points.push_back(p[0][1][0][2]);
						points.push_back(p[0][1][1][0]);points.push_back(p[0][1][1][1]);points.push_back(p[0][1][1][2]);
						points.push_back(p[1][1][1][0]);points.push_back(p[1][1][1][1]);points.push_back(p[1][1][1][2]);
						points.push_back(p[1][1][0][0]);points.push_back(p[1][1][0][1]);points.push_back(p[1][1][0][2]);
						*/
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
	static float rot=0;
	glRotatef(rot, 0, 1, 0);
	rot += 0.1;

//	camera.setupGL(global_xf * point(0,0,0), 1.7);
//    glPushMatrix();
//    glMultMatrixd(global_xf);


	glPushMatrix();
		glColor3f(0, 1, 0);
		glBegin(GL_LINES);
			glVertex3f(0, 0, 0);
			glVertex3f(10, 0, 0);

			glVertex3f(0, 0, 0);
			glVertex3f(0, 10, 0);

			glVertex3f(0, 0, 0);
			glVertex3f(0, 0, 10);
		glEnd();
	glPopMatrix();

	glPushMatrix();
		glColor3f(1, 0, 0);

		glEnableClientState(GL_VERTEX_ARRAY);
		glVertexPointer(3, GL_FLOAT, 0, &points[0]);
		glDrawArrays(GL_POINTS, 0, points.size()/3);
		glDisableClientState(GL_VERTEX_ARRAY);

/*
		for(int y=0; y<GRID_SIZE_Y; y++)
			for(int z=0; z<GRID_SIZE_Z; z++)
				for(int x=0; x<GRID_SIZE_X; x++)
					if(volume[x][y][z])
					{
						//glPushMatrix();
						//glTranslatef(-GRID_SIZE_X/2.0 + x*cubeSize -cubeSize/2,
						//			 -GRID_SIZE_Y/2.0 + y*cubeSize -cubeSize/2,
						//			 -GRID_SIZE_Z/2.0 + z*cubeSize -cubeSize/2);
						//glTranslatef(x*cubeSize,
						//			 z*cubeSize,
						//			 y*cubeSize);
						//	glutSolidCube(cubeSize);
						//glPopMatrix();
					}
*/
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

// Set the view...
void resetview()
{
    camera.stopspin();
    if (!thexform.read(xffilename)) thexform = xform();

    //update_bsph();
    global_xf = xform::trans(0, 0, -5.0f * 1.7) *
	xform::trans(-point(0,0,0));

    if (thexform.read(xffilename)) {
	global_xf = thexform;
	thexform = xform();
    }
}

void mouse(int button, int state, int x, int y)
{
    static timestamp last_click_time;
    static unsigned last_click_buttonstate = 0;
    static float doubleclick_threshold = 0.25f;

    if (glutGetModifiers() & GLUT_ACTIVE_CTRL)
	buttonstate |= (1 << 30);
    else
	buttonstate &= ~(1 << 30);

    if (state == GLUT_DOWN) {
	buttonstate |= (1 << button);
	if (buttonstate == last_click_buttonstate &&
	    now() - last_click_time < doubleclick_threshold) {
	    //doubleclick(button, x, y);
	    last_click_buttonstate = 0;
	} else {
	    last_click_time = now();
	    last_click_buttonstate = buttonstate;
	}
    } else {
	buttonstate &= ~(1 << button);
    }

    //mousemotionfunc(x, y);
	motion(x, y);
}

void motion(int x, int y)
{
/*
    static const Mouse::button physical_to_logical_map[] = {
	Mouse::NONE, Mouse::ROTATE, Mouse::MOVEXY, Mouse::MOVEZ,
	Mouse::MOVEZ, Mouse::MOVEXY, Mouse::MOVEXY, Mouse::MOVEXY,
    };

    Mouse::button b = Mouse::NONE;
    if (buttonstate & (1 << 3))
	b = Mouse::WHEELUP;
    else if (buttonstate & (1 << 4))
	b = Mouse::WHEELDOWN;
    else if (buttonstate & (1 << 30))
	b = Mouse::LIGHT;
    else
	b = physical_to_logical_map[buttonstate & 7];


	xform tmp_xf = global_xf * thexform;
	camera.mouse(x, y, b,
		     tmp_xf * point(0,0,0),
		     1.7,
		     tmp_xf);
	thexform = inv(global_xf) * tmp_xf;
	//update_bsph();
    
    if (b != Mouse::NONE)
	//need_redraw();
*/
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

void Cleanup(int iExitCode)
{
	
}
