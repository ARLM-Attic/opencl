#include "cpuRasterizer.h"

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

CPU_Rasterizer r;

int main(int argc, const char* argv[]) {
	r.readInput("../datasets/d1.txt", true);
	r.initializeConstraints();
	r.clearVolume();
	r.stSimplex();
	r.fillVolume();

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

	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
}

void initDrawArray()
{
	double cubeSize = 2.0/((double)GRID_SIZE_X);
	for(int y=0; y<GRID_SIZE_Y; y++)
		for(int z=0; z<GRID_SIZE_Z; z++)
			for(int x=0; x<GRID_SIZE_X; x++)
				if(r.volume[x][y][z])
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