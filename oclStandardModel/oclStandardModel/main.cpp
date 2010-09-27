#include <ctime>
#include <iostream>
#include <fstream>
#include <ctime>

#include <oclUtils.h>
#include "OpenCL.h"

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
int		volume[GRID_SIZE_X][GRID_SIZE_Y][GRID_SIZE_Z];
Buffer*	volume_d;
/*********************************************/
/* GL *****************************************/
cl_int ciErrNum;

GLuint pbo = 0;                 // OpenGL pixel buffer object

unsigned int width = 512, height = 512;
size_t gridSize[2] = {width, height};
void initPixelBuffer();

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
		c_size = (N+1)*CONSTRAINTS;

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
		cout << "Allocating simplices matrix..." << endl;
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


	cout << "Performing computation on CPU" << endl;
	
	clock_t cpu_b = clock();
	stSimplexCPU(simplices, constraints, nckv, nckRows, SIMPLICES, (int*)volume, GRID_SIZE_X, GRID_SIZE_Y, GRID_SIZE_Z);
	clock_t cpu_a = clock();


	FILE* volOut = fopen("volume.txt", "w");
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
	initPixelBuffer();
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

// Initialize GL
//*****************************************************************************
void initPixelBuffer()
{
     ciErrNum = CL_SUCCESS;

    if (pbo) {
        // delete old buffer
        //clReleaseMemObject(pbo_cl);
        glDeleteBuffersARB(1, &pbo);
    }

    // create pixel buffer object for display
    glGenBuffersARB(1, &pbo);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
	glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, width * height * sizeof(GLubyte) * 4, 0, GL_STREAM_DRAW_ARB);
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

	pbo_cl = cl.createBuffer(width * height * sizeof(GLubyte) * 4, CL_MEM_WRITE_ONLY);

    // calculate new grid size
	gridSize[0] = shrRoundUp(LOCAL_SIZE_X,width);
	gridSize[1] = shrRoundUp(LOCAL_SIZE_Y,height);

	stRender.arg(0, pbo_cl->getMem()).arg(1, width).arg(2, height).arg(3, volume_d->getMem());
	stRender.arg(4, GRID_SIZE_X).arg(5, GRID_SIZE_Y).arg(6, GRID_SIZE_Z);
}

// render image using OpenCL
//*****************************************************************************
void render()
{
    ciErrNum = CL_SUCCESS;

    // execute OpenCL kernel, writing results to PBO
    size_t localSize[] = {LOCAL_SIZE_X,LOCAL_SIZE_Y};

	stRender.local(localSize[0], localSize[1]).global(gridSize[0], gridSize[1]);
	stRender.run();

	// Explicit Copy 
	// map the PBO to copy data from the CL buffer via host
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);    

	// map the buffer object into client's memory
	GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
	pbo_cl->read(ptr, sizeof(float) * height * width, 0, CL_TRUE);
	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); 
}

// Display callback for GLUT main loop
//*****************************************************************************
void DisplayGL()
{
    // use OpenGL to build view matrix
    GLfloat modelView[16];
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glRotatef(-viewRotation[0], 1.0, 0.0, 0.0);
    glRotatef(-viewRotation[1], 0.0, 1.0, 0.0);
    glTranslatef(-viewTranslation[0], -viewTranslation[1], -viewTranslation[2]);
    glGetFloatv(GL_MODELVIEW_MATRIX, modelView);
    glPopMatrix();

    invViewMatrix[0] = modelView[0]; invViewMatrix[1] = modelView[4]; invViewMatrix[2] = modelView[8]; invViewMatrix[3] = modelView[12];
    invViewMatrix[4] = modelView[1]; invViewMatrix[5] = modelView[5]; invViewMatrix[6] = modelView[9]; invViewMatrix[7] = modelView[13];
    invViewMatrix[8] = modelView[2]; invViewMatrix[9] = modelView[6]; invViewMatrix[10] = modelView[10]; invViewMatrix[11] = modelView[14];

     // process 
    render();

    // draw image from PBO
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_DEPTH_TEST);
    glRasterPos2i(0, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo);
    glDrawPixels(width, height, GL_RGBA, GL_UNSIGNED_BYTE, 0);
    glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

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

void mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN)
		buttonState |= 1<<button;
	else if (state == GLUT_UP)
		buttonState = 0;

	ox = x; 
	oy = y;
	glutPostRedisplay();
}

void motion(int x, int y)
{
	float dx, dy;
	dx = x - ox;
	dy = y - oy;

	if (buttonState == 3) {
		// left+middle = zoom
		viewTranslation[2] += dy / 100.0;
	} 
	else if (buttonState & 2) {
		// middle = translate
		viewTranslation[0] += dx / 100.0;
		viewTranslation[1] -= dy / 100.0;
	}
	else if (buttonState & 1) {
		// left = rotate
		viewRotation[0] += dy / 5.0;
		viewRotation[1] += dx / 5.0;
	}

	ox = x; 
	oy = y;
	glutPostRedisplay();
}

void Reshape(int x, int y)
{
	width = x; height = y;
	initPixelBuffer();

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