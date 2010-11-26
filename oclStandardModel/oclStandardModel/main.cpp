#include "cpuRasterizer.h"
#include "gpuRasterizer.h"

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
bool useGpuResults = false;

int main(int argc, const char* argv[]) {
	const char* inputFile = "../datasets/d1.txt";
	const bool isHeightMap = true;
	//const char* inputFile = "simplices_in.txt";
	//const bool isHeightMap = false;


	cout << "Starting CPU version..." << endl;
	cpuR.readInput(inputFile, isHeightMap);
	cpuR.initializeConstraints();
	cpuR.clearVolume();
	cpuR.stSimplex();
	cpuR.fillVolume();
	cout << "CPU version finished!" << endl;

	cout << "Starting GPU version..." << endl;
	gpuR.readInput(inputFile, isHeightMap);
	gpuR.initializeConstraints();
	gpuR.clearVolume();
	gpuR.buildProgram();
	gpuR.setWorksize(LOCAL_WORKSIZE, shrRoundUp(LOCAL_WORKSIZE, gpuR.getNumSimplices()));
	gpuR.stSimplex();
	gpuR.readResults();
	gpuR.fillVolume();
	cout << "GPU version finished!" << endl;

	InitGL(argc, argv);
	initDrawArray();
	glutMainLoop();
	return 0;
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
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -10.0, 10.0); 

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glPushMatrix();
		static float rot=0;
		glRotatef(rot, 0, 1, 0);
		rot += 0.1;

		glTranslatef(-0.9, -0.9, -0.9);
		glRotatef(5, 1, 0, 1);

		// process 
		render();
	glPopMatrix();
	glColor3f(0.9, 0.9, 0.9);
	const char* label;
	if(useGpuResults) label = "GPU Rasterizer";
	else label = "CPU Rasterizer";
	drawText(0.6, 0.9, 0, GLUT_BITMAP_HELVETICA_12, label);

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
}

// GL Idle time callback
//*****************************************************************************
void Idle()
{

    glutPostRedisplay();
}