#define _USE_GPU_

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
vector<float> points;
vector<float> axis;

unsigned int width = 800, height = 600;
unsigned int iGLUTWindowHandle;

void render();

void initDrawArray();
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
char* g_Filename = NULL;
bool g_UseGpuResults = false;
bool g_IsHeightMap = false;

void TW_CALL GetRunOnGpuCB(void *value, void *clientData);
void TW_CALL SetRunOnGpuCB(const void *value, void *clientData);

void TW_CALL SetAutoRotateCB(const void *value, void *clientData);
void TW_CALL GetAutoRotateCB(void *value, void *clientData);

void TW_CALL OpenFileCB(void *clientData);

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

	// offToTriangles("neptune.off", "neptuneF.txt", 160);

	//datasets: ../datasets/d1.txt true
	//			hand.txt false
	//			neptune.txt false

	InitGL(argc, argv);
	glutMainLoop();
	return 0;
}

void openFile(const char* inputFile, const bool isHeightMap) {
	cout << "Opening " << inputFile << "..." << endl;

	cpuR.readInput(inputFile, isHeightMap);
	cpuR.initializeConstraints();
	cpuR.clearVolume();
	size_t time = clock();
	cpuR.stSimplex();
	time = clock() - time;
	cpuR.fillVolume();
	cout << "CPU version took " << time << "ms!" << endl;

	ompR.readInput(inputFile, isHeightMap);
	ompR.initializeConstraints();
	ompR.clearVolume();
	time = clock();
	ompR.stSimplex();
	time = clock() - time;
	ompR.fillVolume();
	cout << "OpenMP version took " << time << "ms!" << endl;

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
	cout << "GPU version took " << time << "ms!" << endl;
#endif

	initDrawArray();
}

void initializeTweakBar() {
	TwInit(TW_OPENGL, NULL);

	bar = TwNewBar("Config");
	TwDefine(" Config position='550 345' size='225 230' color='150 150 150' ");

	TwAddVarRW(bar, "Zoom", TW_TYPE_FLOAT, &g_Zoom, 
		" min=0.01 max=2.5 step=0.01 keyIncr=z keyDecr=Z help='Scale the object (1=original size).' ");
	TwAddVarRW(bar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation, 
		" label='Object rotation' open help='Change the object orientation.' ");
	// Add callback to toggle auto-rotate mode (callback functions are defined above).
	TwAddVarCB(bar, "AutoRotate", TW_TYPE_BOOL32, SetAutoRotateCB, GetAutoRotateCB, NULL, 
		" label='Auto-rotate' key=space help='Toggle auto-rotate mode.' ");
	TwAddVarCB(bar, "RunOnGPU", TW_TYPE_BOOLCPP, SetRunOnGpuCB, GetRunOnGpuCB, NULL, 
		" label='GPU' key=c help='Toggle run mode.' ");

	TwCopyCDStringToClientFunc(CopyCDStringToClient);
    TwAddVarRW(bar, "Filename", TW_TYPE_CDSTRING, &g_Filename, 
               " label='Filename' group=OpenFile help='Mesh to be rasterized.' ");
	TwAddVarCB(bar, "IsHeightMap", TW_TYPE_BOOLCPP, SetIsHeightMapCB, GetIsHeightMapCB, NULL, 
		" label='HeightMap' group=OpenFile key=h true='TRUE' false='FALSE' help='Toggle input type.' ");
    // Add a button to create a new bar using the title
    TwAddButton(bar, "OpenFileButton", OpenFileCB, &g_Filename, 
                " label='Open' group=OpenFile key=o help='Open file.' ");
    // Set the group label & separator
    TwDefine(" Config/OpenFile label='Open a File' help='This example demonstates different use of std::string variables.' ");
}

void InitGL(int argc, const char **argv)
{
	// initialize GLUT 
	glutInit(&argc, (char **)argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - width/2, 
		glutGet(GLUT_SCREEN_HEIGHT)/2 - height/2);
	glutInitWindowSize(width, height);
	iGLUTWindowHandle = glutCreateWindow("oclStandardModel Rasterizer");

	// register glut callbacks
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

void initDrawArray()
{
	points.clear();
	double cubeSize = 2.0/((double)GRID_SIZE_X);
	for(int y=0; y<GRID_SIZE_Y; y++)
		for(int z=0; z<GRID_SIZE_Z; z++)
			for(int x=0; x<GRID_SIZE_X; x++)
				if(
#ifdef _USE_GPU_
					(g_UseGpuResults && gpuR.volume[x][y][z]) || 
#endif
					(!g_UseGpuResults && cpuR.volume[x][y][z]))
				{
					points.push_back((x+0.5)*cubeSize);
					points.push_back((z+0.5)*cubeSize);
					points.push_back((y+0.5)*cubeSize);
				}
}

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
			glColor3f(0.95, 0.2, 0.2);
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
	float mat[4*4]; // rotation matrix

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0); 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
		if(g_AutoRotate)  {
			float axis[3] = { 0, 1, 0 };
			float angle = (float)(glutGet(GLUT_ELAPSED_TIME)-g_RotateTime)/1000.0f;
			float quat[4];
			SetQuaternionFromAxisAngle(axis, angle, quat);
			MultiplyQuaternions(g_RotateStart, quat, g_Rotation);
		}
		ConvertQuaternionToMatrix(g_Rotation, mat);
		glMultMatrixf(mat);
		glTranslatef(-0.9, -0.9, -0.9);
		glScalef(g_Zoom, g_Zoom, g_Zoom);

		render();
	glPopMatrix();

	glColor3f(0.9, 0.9, 0.9);
	const char* label;
	if(g_UseGpuResults) label = "GPU Rasterizer";
	else label = "CPU Rasterizer";
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
//*****************************************************************************
void Idle()
{
    glutPostRedisplay();
}

void TW_CALL GetRunOnGpuCB(void *value, void *clientData) {
	*(bool *)(value) = g_UseGpuResults;
}

void TW_CALL SetRunOnGpuCB(const void *value, void *clientData) {
	g_UseGpuResults = *(bool *)(value);
	initDrawArray();
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

// Callback function to create a bar with a given title
void TW_CALL OpenFileCB(void *clientData)
{
    char **filename = (char **)(clientData);
	
	if(*filename != NULL)
		openFile(*filename, g_IsHeightMap);
}
