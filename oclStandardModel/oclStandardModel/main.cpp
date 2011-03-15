/* Federal University of Rio Grande do Sul (UFRGS)
 * Instituto de Informatica
 *
 * C++/OpenCL/OpenMP Rasterizer. Receives either a heightmap or a mesh of
 * triangles as input and rasterize it using the "Standard Model" presented in
 *
 *		Andres, E. Discrete linear objects in dimension n: the standard model.
 *		Graphical Models 65 (2003) 92–111.
 *
 * Requirements:
 *	- OCLpp
 *      Win32: http://opencl.codeplex.com/releases
 *      Source: http://opencl.codeplex.com/SourceControl/list/changesets
 *
 *  - AntTweakBar
 *      Multi-platform: http://www.antisphere.com/Wiki/tools:anttweakbar?sb=tools:anttweakbar
 *
 *	- Nvidia GPU Computing SDK / CUDA SDK
 *      Multi-platform: http://developer.nvidia.com/object/gpucomputing.html
 *
 *
 * Developed by Bruno Jurkovski and Leonardo Chatain (2010)
 * Advised by Leandro A. F. Fernandes and Manuel M. Oliveira
 *
 */

// Use this definition if you have an OpenCL enabled GPU
//#define _USE_GPU_
// Use this when running benchmarks (to save a "benchmarks.csv" file with the results)
//#define _RUN_BENCHMARK

#include <cstring>
#include <cstdlib>
#include <cstdarg>
#include <vector>

#include "AntTweakBar.h"
#include "tweakAux.h"

#include "cpuRasterizer.h"
#include "ompRasterizer.h"
#ifdef _USE_GPU_
#include "gpuRasterizer.h"
#endif

#include <GL/glut.h>

#include "offLoader.h"

using namespace std;

/* GL ****************************************/
vector<float> points;	// Points that will be rasterized
vector<float> axis;		// Axis points to be rasterized

// Window Dimensions
unsigned int width = 800, height = 600;
unsigned int iGLUTWindowHandle;

// Misc Functions
void render();

// Callbacks
void initDrawArray(const Rasterizer& rasterizer);
void InitGL(int argc, const char** argv);
void DisplayGL();
void Reshape(int w, int h);
void Idle(void);
/*********************************************/
/* TweakBar **********************************/
void initializeTweakBar();

TwBar* bar;
// Shapes scale
float g_Zoom = 0.8;
// Shape orientation (stored as a quaternion)
float g_Rotation[] = {0.0f, 0.0f, 0.0f, 1.0f};
// Auto rotate
int g_AutoRotate = 0;
int g_RotateTime = 0;
float g_RotateStart[] = {0.0f, 0.0f, 0.0f, 1.0f};
char* g_Filename = NULL;		// Input filename
bool g_IsHeightMap = false;		// Kind of input (height map or mesh of triangles)
char* g_ResultFilename = NULL;	// Filename of an already computed output, for comparison

typedef enum {
	CPU_RESULTS,
	OMP_RESULTS,
	GPU_RESULTS
} RasterizerVersion;

TwEnumVal resultsEV[] = {{CPU_RESULTS, "CPU"}, {OMP_RESULTS, "OpenMP"}, {GPU_RESULTS, "GPU"}};
TwType resultsType;							// TweakBar handler of type "RasterizerVersion"
RasterizerVersion version = CPU_RESULTS;	// Version appearing on the screen

// Getters and Setters for the interface (TweakBar)
void TW_CALL GetVersionCB(void *value, void *clientData);
void TW_CALL SetVersionCB(const void *value, void *clientData);

void TW_CALL SetAutoRotateCB(const void *value, void *clientData);
void TW_CALL GetAutoRotateCB(void *value, void *clientData);

void TW_CALL OpenFileCB(void *clientData);
void TW_CALL CompareResultsCB(void *clientData);

void TW_CALL GetIsHeightMapCB(void *value, void *clientData);
void TW_CALL SetIsHeightMapCB(const void *value, void *clientData);
/*********************************************/
/* Rasterizer ********************************/
CPU_Rasterizer cpuR;
OMP_Rasterizer ompR;
#ifdef _USE_GPU_
GPU_Rasterizer gpuR;
#endif

void openFile(const char* inputFile, const bool isHeightMap);
/*********************************************/

int main(int argc, const char* argv[]) {
	initializeTweakBar();

	//offToTriangles("neptune.off", "neptuneF.txt", 160);

	//datasets: ../datasets/d1.txt true
	//			hand.txt false
	//			neptune.txt false

	char* inputFile;
	bool inputIsHeightMap = false;

	if(argc >= 4)
		inputIsHeightMap = !strcmp(argv[3], "--heightmap");

	if(argc >= 3)
		if(!strcmp(argv[1], "-f")) {
			inputFile = (char*) argv[2];
			openFile(inputFile, inputIsHeightMap);
		}

	// Code to run benchmarks (don't need to use the graphical interface)
#ifdef _RUN_BENCHMARK
	const char* files[] = {"hand.txt", "neptune.txt", "../datasets/d1.txt"};
	const bool hmaps[] = {false, false, true};

	printf("Running benchmarks...\n");
	for(int i=0; i<3; i++) {
		for(int j=0; j<15; j++)
			openFile(files[i], hmaps[i]);
	}
	printf("Finished!\n");
#endif

	InitGL(argc, argv);
	glutMainLoop();
	return 0;
}

//
//	Create each one of the Rasterizers (CPU, OpenMP and GPU, if available)
//	and run it for the given input.
//
void openFile(const char* inputFile, const bool isHeightMap) {
#ifdef _RUN_BENCHMARK
	cout << "Opening " << inputFile << "..." << endl;
	FILE* bFile = fopen("benchmarks.csv", "a+");
#endif

	cpuR.readInput(inputFile, isHeightMap);
	cpuR.initializeConstraints();
	cpuR.clearVolume();
	size_t time = clock();
	cpuR.stSimplex();
	time = clock() - time;
	cpuR.fillVolume();

#ifdef _RUN_BENCHMARK
	cout << "CPU version took " << time << "ms!" << endl;
	fprintf(bFile, "%s,CPU,%d\n", inputFile, time);
#endif

	ompR.readInput(inputFile, isHeightMap);
	ompR.initializeConstraints();
	ompR.clearVolume();
	time = clock();
	ompR.stSimplex();
	time = clock() - time;
	ompR.fillVolume();

#ifdef _RUN_BENCHMARK
	cout << "OpenMP version took " << time << "ms!" << endl;
	fprintf(bFile, "%s,OpenMP,%d\n", inputFile, time);
#endif

#ifdef _USE_GPU_
	gpuR.readInput(inputFile, isHeightMap);
	gpuR.initializeConstraints();
	gpuR.clearVolume();
	gpuR.buildProgram();
	gpuR.setWorksize(LOCAL_WORKSIZE, shrRoundUp(LOCAL_WORKSIZE, gpuR.getNumSimplices()));
	time = clock();
	gpuR.stSimplex();
	gpuR.readResults();
	time = clock() - time;
	gpuR.fillVolume();

	#ifdef _RUN_BENCHMARK
		cout << "GPU version took " << time << "ms!" << endl;
		fprintf(bFile, "%s,GPU,%d\n", inputFile, time);
	#endif
#endif

#ifdef _RUN_BENCHMARK
	fclose(bFile);
#endif

	initDrawArray(cpuR);
}

//
//	Creation of the interface; Details about each function can be found here:
//	http://www.antisphere.com/Wiki/tools:anttweakbar?sb=tools:anttweakbar
//
void initializeTweakBar() {
	TwInit(TW_OPENGL, NULL);

	bar = TwNewBar("Config");
	TwDefine(" Config position='550 345' size='225 230' color='150 150 150' ");

	TwAddVarRW(bar, "Zoom", TW_TYPE_FLOAT, &g_Zoom, " min=0.01 max=2.5 step=0.01 keyIncr=z keyDecr=Z ");
	TwAddVarRW(bar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation, " label='Object rotation' open ");
	TwAddVarCB(bar, "AutoRotate", TW_TYPE_BOOL32, SetAutoRotateCB, GetAutoRotateCB, NULL, " label='Auto-rotate' key=space ");
	resultsType = TwDefineEnum("ResultsType", resultsEV, 3);
	TwAddVarCB(bar, "Version", resultsType, SetVersionCB, GetVersionCB, NULL, " keyIncr=v keyDecr=V");

	TwCopyCDStringToClientFunc(CopyCDStringToClient);
    TwAddVarRW(bar, "Filename", TW_TYPE_CDSTRING, &g_Filename,  " label='Filename' group=OpenFile ");
	TwAddVarCB(bar, "IsHeightMap", TW_TYPE_BOOLCPP, SetIsHeightMapCB, GetIsHeightMapCB, NULL, 
								" label='HeightMap' group=OpenFile key=h true='TRUE' false='FALSE' ");
    TwAddButton(bar, "OpenFileButton", OpenFileCB, &g_Filename, " label='Open' group=OpenFile key=o ");
    TwDefine(" Config/OpenFile label='Open a File' ");

	TwAddVarRW(bar, "Result Filename", TW_TYPE_CDSTRING, &g_ResultFilename,  " label='Results Filename' group=CheckResult ");
	TwAddButton(bar, "CompareResultsButton", CompareResultsCB, &g_ResultFilename, " label='Compare' group=CheckResult key=p ");
}

void InitGL(int argc, const char **argv)
{
	// Initialize GLUT 
	glutInit(&argc, (char **)argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - width/2, 
		glutGet(GLUT_SCREEN_HEIGHT)/2 - height/2);
	glutInitWindowSize(width, height);
	iGLUTWindowHandle = glutCreateWindow("oclStandardModel Rasterizer");

	// Register glut callbacks
	glutDisplayFunc(DisplayGL);
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);

	// Set GLUT event callbacks to AntTweakBar
	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	TwGLUTModifiersFunc(glutGetModifiers);
}

//
//	Prepare the volume to be drawn on the screen.
//	Receives a Rasterizer and use its volume (internal attribute)
//	to fill the points vector.
//
void initDrawArray(const Rasterizer& rasterizer)
{
	points.clear();
	double cubeSizeX = 2.0/((double)GRID_SIZE_X);
	double cubeSizeY = 2.0/((double)GRID_SIZE_Y);
	double cubeSizeZ = 2.0/((double)GRID_SIZE_Z);
	for(int y=0; y<GRID_SIZE_Y; y++)
		for(int z=0; z<GRID_SIZE_Z; z++)
			for(int x=0; x<GRID_SIZE_X; x++)
				if(rasterizer.volume[x][y][z])
				{
					points.push_back((x+0.5)*cubeSizeX);
					points.push_back((z+0.5)*cubeSizeZ);
					points.push_back((y+0.5)*cubeSizeY);
				}
}

//
//	Aux function to render a text on the screen.
//	Receives it's position, font and content and print rasterize it.
//	(the content works like the printf function: a string with the format
//	 followed by the referenced variables).
//
void drawText(const float x, const float y, const float z, void* font, const char* format, ...)
{
	int i, len;
	char str[500];

	glPushAttrib(GL_DEPTH_BUFFER_BIT | GL_LIGHTING_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	va_list params;
	va_start(params, format);
	vsprintf(str, format, params);

	glRasterPos3f(x, y, z);
	for (i = 0, len = strlen(str); i < len; i++)
		glutBitmapCharacter(font, (int)str[i]);

	va_end(params);
	glPopAttrib();
}


//
// Render image using OpenCL
//
void render()
{
	glClearColor(0, 0, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);

	glPushMatrix();
		glPushMatrix();
			glColor3f(0, 1, 0);
			glBegin(GL_LINES);
				//
				//	Draw X,Y,Z axis.
				//
				glVertex3f(0,0,0);
				glVertex3f(10,0,0);
				glVertex3f(0,0,0);
				glVertex3f(0,10,0);
				glVertex3f(0,0,0);
				glVertex3f(0,0,10);
			glEnd();
		glPopMatrix();

	if(points.size() > 0) {
		glPushMatrix();
				glColor3f(0.95, 0.2, 0.2);
				//
				//	Draw the points array
				//
				glEnableClientState(GL_VERTEX_ARRAY);
				glVertexPointer(3, GL_FLOAT, 0, &points[0]);
				glDrawArrays(GL_POINTS, 0, points.size()/3);
				glDisableClientState(GL_VERTEX_ARRAY);
		glPopMatrix();
	}
}

//
// Display callback for GLUT main loop
//
void DisplayGL()
{
	float mat[4*4]; // rotation matrix

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0); 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
		//
		// Compute scene rotation based on a quaternion
		//
		if(g_AutoRotate)  {
			float axis[3] = { 0, 1, 0 };
			float angle = (float)(glutGet(GLUT_ELAPSED_TIME)-g_RotateTime)/1000.0f;
			float quat[4];
			SetQuaternionFromAxisAngle(axis, angle, quat);
			MultiplyQuaternions(g_RotateStart, quat, g_Rotation);
		}
		ConvertQuaternionToMatrix(g_Rotation, mat);
		glMultMatrixf(mat);
		//glTranslatef(-0.9, -0.9, -0.9);
		//glTranslatef(0, -0.9, 0);
		glScalef(g_Zoom, g_Zoom, g_Zoom);

		render();
	glPopMatrix();

	glColor3f(0.9, 0.9, 0.9);
	const char* label;
	if (version == CPU_RESULTS) label = "CPU Rasterizer";
	else if(version == OMP_RESULTS) label = "OpenMP Rasterizer";
	else if(version == GPU_RESULTS) label = "GPU Rasterizer";
	drawText(0.6, 0.9, 0, GLUT_BITMAP_HELVETICA_12, label);

	TwDraw();
	// flip backbuffer to screen
	glutSwapBuffers();
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

	TwWindowSize(width, height);
}

// GL Idle time callback
void Idle()
{
    glutPostRedisplay();
}

/* TweakBar Callbacks */
void TW_CALL GetVersionCB(void *value, void *clientData) {
	*(RasterizerVersion *)(value) = version;
}

void TW_CALL SetVersionCB(const void *value, void *clientData) {
	version = *(RasterizerVersion *)(value);
	if(version == CPU_RESULTS) initDrawArray(cpuR);
	else if(version == OMP_RESULTS) initDrawArray(ompR);
#ifdef _USE_GPU_
	else if(version == GPU_RESULTS) initDrawArray(gpuR);
#endif
}

void TW_CALL GetIsHeightMapCB(void *value, void *clientData) {
	*(bool *)(value) = g_IsHeightMap;
}

void TW_CALL SetIsHeightMapCB(const void *value, void *clientData) {
	g_IsHeightMap = *(bool *)(value);
}


//  Callback function called when the 'AutoRotate' variable value of the tweak bar has changed
void TW_CALL SetAutoRotateCB(const void *value, void *clientData)
{
	(void)clientData; // unused

	g_AutoRotate = *(const int *)(value); // copy value to g_AutoRotate
	if(g_AutoRotate != 0) 
	{
		// init rotation
		g_RotateTime = glutGet(GLUT_ELAPSED_TIME);
		g_RotateStart[0] = g_Rotation[0];
		g_RotateStart[1] = g_Rotation[1];
		g_RotateStart[2] = g_Rotation[2];
		g_RotateStart[3] = g_Rotation[3];

		// make Rotation variable read-only
		TwDefine(" Config/ObjRotation readonly ");
	}
	else
		// make Rotation variable read-write
		TwDefine(" Config/ObjRotation readwrite ");
}


//  Callback function called by the tweak bar to get the 'AutoRotate' value
void TW_CALL GetAutoRotateCB(void *value, void *clientData)
{
	(void)clientData; // unused
	*(int *)(value) = g_AutoRotate; // copy g_AutoRotate to value
}

// Callback function to open a input file
void TW_CALL OpenFileCB(void *clientData)
{
	char **filename = (char **)(clientData);

	if(*filename != NULL)
		openFile(*filename, g_IsHeightMap);
}

// Callback function to compare the current results with the ones stored in a file
void TW_CALL CompareResultsCB(void *clientData)
{
	char **filename = (char **)(clientData);

	if(*filename != NULL) {
		if (version == CPU_RESULTS) cpuR.compareResults(*filename);
		else if(version == OMP_RESULTS) ompR.compareResults(*filename);
#ifdef _USE_GPU_
		else if(version == GPU_RESULTS) gpuR.compareResults(*filename);
#endif
	}
}
