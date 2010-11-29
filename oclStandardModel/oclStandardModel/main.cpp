#include <AntTweakBar.h>
#include "cpuRasterizer.h"
#include "gpuRasterizer.h"
#include "ompRasterizer.h"

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
void KeyboardGL(unsigned char key, int x, int y);
void Reshape(int w, int h);
void motion(int x, int y);
void mouseFunction(int button, int state, int x, int y);
void Idle(void);
/*********************************************/

CPU_Rasterizer cpuR;
GPU_Rasterizer gpuR;
OMP_Rasterizer ompR;
bool useGpuResults = false;

/* TweakBar **********************************/
void initializeTweakBar();

TwBar* bar;
// Shapes scale
float g_Zoom = 0.8;
// Shape orientation (stored as a quaternion)
float g_Rotation[] = { 0.0f, 0.0f, 0.0f, 1.0f };
// Auto rotate
int g_AutoRotate = 0;
int g_RotateTime = 0;
float g_RotateStart[] = { 0.0f, 0.0f, 0.0f, 1.0f };

void TW_CALL GetRunOnGpuCB(void *value, void *clientData);
void TW_CALL SetRunOnGpuCB(const void *value, void *clientData);

void TW_CALL SetAutoRotateCB(const void *value, void *clientData);
void TW_CALL GetAutoRotateCB(void *value, void *clientData);

void SetQuaternionFromAxisAngle(const float *axis, float angle, float *quat);
void ConvertQuaternionToMatrix(const float *quat, float *mat);
void MultiplyQuaternions(const float *q1, const float *q2, float *qout);
/*********************************************/

int main(int argc, const char* argv[]) {
	initializeTweakBar();

	//const char* inputFile = "../datasets/d1.txt";
	//const bool isHeightMap = true;
	//const char* inputFile = "simplices_in2.txt";
	//const bool isHeightMap = false;
	const char* inputFile = "ds.txt";
	const bool isHeightMap = false;

	cout << "Starting CPU version..." << endl;
	cpuR.readInput(inputFile, isHeightMap);
	cpuR.initializeConstraints();
	cpuR.clearVolume();
	size_t time = clock();
	cpuR.stSimplex();
	cpuR.fillVolume();
	time = clock() - time;
	cout << "CPU version finished! " << time << endl;

	cout << "Starting OMP version..." << endl;
	ompR.readInput(inputFile, isHeightMap);
	ompR.initializeConstraints();
	ompR.clearVolume();
	time = clock();
	ompR.stSimplex();
	ompR.fillVolume();
	time = clock() - time;
	cout << "OMP version finished! " << time << endl;

	cout << "Starting GPU version..." << endl;
	gpuR.readInput(inputFile, isHeightMap);
	gpuR.initializeConstraints();
	gpuR.clearVolume();
	gpuR.buildProgram();
	gpuR.setWorksize(LOCAL_WORKSIZE, shrRoundUp(LOCAL_WORKSIZE, gpuR.getNumSimplices()));
	time = clock();
	gpuR.stSimplex();
	gpuR.readResults();
	gpuR.fillVolume();
	time = clock() - time;
	cout << "GPU version finished! " << time << endl;

	InitGL(argc, argv);
	initDrawArray();
	glutMainLoop();
	return 0;
}

void initializeTweakBar() {
	TwInit(TW_OPENGL, NULL);

	bar = TwNewBar("Config");
	TwDefine(" Config position='550 415' size='225 160' color='150 150 150' ");

	TwAddVarRW(bar, "Zoom", TW_TYPE_FLOAT, &g_Zoom, 
		" min=0.01 max=2.5 step=0.01 keyIncr=z keyDecr=Z help='Scale the object (1=original size).' ");
	TwAddVarRW(bar, "ObjRotation", TW_TYPE_QUAT4F, &g_Rotation, 
		" label='Object rotation' open help='Change the object orientation.' ");
	// Add callback to toggle auto-rotate mode (callback functions are defined above).
	TwAddVarCB(bar, "AutoRotate", TW_TYPE_BOOL32, SetAutoRotateCB, GetAutoRotateCB, NULL, 
		" label='Auto-rotate' key=space help='Toggle auto-rotate mode.' ");
	TwAddVarCB(bar, "RunOnGPU", TW_TYPE_BOOL8, SetRunOnGpuCB, GetRunOnGpuCB, NULL, 
		" label='GPU' key=c help='Toggle run mode.' ");
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
	glutReshapeFunc(Reshape);
	glutIdleFunc(Idle);

	glutKeyboardFunc(KeyboardGL);
	//glutMouseFunc(mouseFunction);
	//glutMotionFunc(motion);

	// Set GLUT event callbacks
	// - Directly redirect GLUT mouse button events to AntTweakBar
	glutMouseFunc((GLUTmousebuttonfun)TwEventMouseButtonGLUT);
	// - Directly redirect GLUT mouse motion events to AntTweakBar
	glutMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	// - Directly redirect GLUT mouse "passive" motion events to AntTweakBar (same as MouseMotion)
	glutPassiveMotionFunc((GLUTmousemotionfun)TwEventMouseMotionGLUT);
	// - Directly redirect GLUT key events to AntTweakBar
	glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
	// - Directly redirect GLUT special key events to AntTweakBar
	glutSpecialFunc((GLUTspecialfun)TwEventSpecialGLUT);
	// - Send 'glutGetModifers' function pointer to AntTweakBar;
	//   required because the GLUT key event functions do not report key modifiers states.
	TwGLUTModifiersFunc(glutGetModifiers);

	glewInit();
}

void initDrawArray()
{
	points.clear();
	double cubeSize = 2.0/((double)GRID_SIZE_X);
	for(int y=0; y<GRID_SIZE_Y; y++)
		for(int z=0; z<GRID_SIZE_Z; z++)
			for(int x=0; x<GRID_SIZE_X; x++)
				if((useGpuResults && gpuR.volume[x][y][z]) || (!useGpuResults && cpuR.volume[x][y][z]))
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
	if(useGpuResults) label = "GPU Rasterizer";
	else label = "CPU Rasterizer";
	drawText(0.6, 0.9, 0, GLUT_BITMAP_HELVETICA_12, label);

	TwDraw();
	// flip backbuffer to screen
	glutSwapBuffers();
	glutPostRedisplay();
}

// Keyboard event handler callback
//*****************************************************************************
void KeyboardGL(unsigned char key, int /*x*/, int /*y*/)
{
	if(key == 'C' || key == 'c') {
		useGpuResults = !useGpuResults;
		initDrawArray();
	}
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

	TwWindowSize(width, height);
}

// GL Idle time callback
//*****************************************************************************
void Idle()
{

    glutPostRedisplay();
}







void TW_CALL GetRunOnGpuCB(void *value, void *clientData) {
	*(bool *)(value) = useGpuResults;
}

void TW_CALL SetRunOnGpuCB(const void *value, void *clientData)
{
	useGpuResults = *(bool *)(value);
	initDrawArray();
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


////////////////////////////////////////////////
//   TWEAK BAR AUX FUNCTIONS (QUATERNIONS)    //
////////////////////////////////////////////////

// Routine to set a quaternion from a rotation axis and angle
// ( input axis = float[3] angle = float  output: quat = float[4] )
void SetQuaternionFromAxisAngle(const float *axis, float angle, float *quat)
{
	float sina2, norm;
	sina2 = (float)sin(0.5f * angle);
	norm = (float)sqrt(axis[0]*axis[0] + axis[1]*axis[1] + axis[2]*axis[2]);
	quat[0] = sina2 * axis[0] / norm;
	quat[1] = sina2 * axis[1] / norm;
	quat[2] = sina2 * axis[2] / norm;
	quat[3] = (float)cos(0.5f * angle);

}


// Routine to convert a quaternion to a 4x4 matrix
// ( input: quat = float[4]  output: mat = float[4*4] )
void ConvertQuaternionToMatrix(const float *quat, float *mat)
{
	float yy2 = 2.0f * quat[1] * quat[1];
	float xy2 = 2.0f * quat[0] * quat[1];
	float xz2 = 2.0f * quat[0] * quat[2];
	float yz2 = 2.0f * quat[1] * quat[2];
	float zz2 = 2.0f * quat[2] * quat[2];
	float wz2 = 2.0f * quat[3] * quat[2];
	float wy2 = 2.0f * quat[3] * quat[1];
	float wx2 = 2.0f * quat[3] * quat[0];
	float xx2 = 2.0f * quat[0] * quat[0];
	mat[0*4+0] = - yy2 - zz2 + 1.0f;
	mat[0*4+1] = xy2 + wz2;
	mat[0*4+2] = xz2 - wy2;
	mat[0*4+3] = 0;
	mat[1*4+0] = xy2 - wz2;
	mat[1*4+1] = - xx2 - zz2 + 1.0f;
	mat[1*4+2] = yz2 + wx2;
	mat[1*4+3] = 0;
	mat[2*4+0] = xz2 + wy2;
	mat[2*4+1] = yz2 - wx2;
	mat[2*4+2] = - xx2 - yy2 + 1.0f;
	mat[2*4+3] = 0;
	mat[3*4+0] = mat[3*4+1] = mat[3*4+2] = 0;
	mat[3*4+3] = 1;
}


// Routine to multiply 2 quaternions (ie, compose rotations)
// ( input q1 = float[4] q2 = float[4]  output: qout = float[4] )
void MultiplyQuaternions(const float *q1, const float *q2, float *qout)
{
	float qr[4];
	qr[0] = q1[3]*q2[0] + q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1];
	qr[1] = q1[3]*q2[1] + q1[1]*q2[3] + q1[2]*q2[0] - q1[0]*q2[2];
	qr[2] = q1[3]*q2[2] + q1[2]*q2[3] + q1[0]*q2[1] - q1[1]*q2[0];
	qr[3]  = q1[3]*q2[3] - (q1[0]*q2[0] + q1[1]*q2[1] + q1[2]*q2[2]);
	qout[0] = qr[0]; qout[1] = qr[1]; qout[2] = qr[2]; qout[3] = qr[3];
}