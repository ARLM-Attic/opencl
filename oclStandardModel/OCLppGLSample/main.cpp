#include "OpenCL.h"
#include <oclUtils.h>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
GLint windowHandle;
float rotate[3];  
bool InitGL(int argc, const char **argv );

cl_int image_width = WINDOW_WIDTH;
cl_int image_height = WINDOW_HEIGHT;
GLuint pbo_source;
GLuint pbo_dest;
unsigned int size_tex_data;
unsigned int num_texels;
unsigned int num_values;
void createPBO(GLuint* pbo);

GLuint tex_screen;
void createTexture( GLuint* tex_name, unsigned int size_x, unsigned int size_y);

int blur_radius = 4;                // radius of 2D convolution performed in post processing step
void processImage();
void renderScene();
void DisplayGL();
void displayImage();
void idle();

ocl::OpenCL cl;
ocl::Launcher kernelLauncher;
cl_mem cl_pbos[2] = {0,0};
size_t szLocalWorkSize[2];
size_t szGlobalWorkSize[2];
cl_int ciErrNum;

#include <iostream>
using namespace std;
int main(int argc, const char **argv)
{
	InitGL(argc, argv);

	createPBO(&pbo_source);
	createPBO(&pbo_dest);

	// create texture for blitting onto the screen
	createTexture(&tex_screen, image_width, image_height);    

	cl_pbos[0] = cl.createBuffer(4 * image_width * image_height, CL_MEM_READ_ONLY);
	cl_pbos[1] = cl.createBuffer(4 * image_width * image_height, CL_MEM_WRITE_ONLY);

	vector<string> files;
	files.push_back("PostprocessGL.cl");

	ocl::Program* prog = cl.createProgram(files);
	prog->build();

	kernelLauncher = prog->createLauncher("postprocess");
	kernelLauncher.arg(cl_pbos[0]).arg(cl_pbos[1]).arg(image_width).arg(image_height);

	glutMainLoop();

	return 0;
}

bool InitGL(int argc, const char **argv )
{
	// init GLUT and GLUT window
	glutInit(&argc, (char**)argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_ALPHA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowPosition (glutGet(GLUT_SCREEN_WIDTH)/2 - WINDOW_WIDTH/2, 
							glutGet(GLUT_SCREEN_HEIGHT)/2 - WINDOW_HEIGHT/2);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	windowHandle = glutCreateWindow("OCLpp/OpenGL post-processing");

	// register GLUT callbacks
	glutDisplayFunc(DisplayGL);
	glutIdleFunc(idle);

	// init GLEW
	glewInit();
	GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"); 

	// default initialization
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glDisable(GL_DEPTH_TEST);

	// viewport
	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	// projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)WINDOW_WIDTH / (GLfloat) WINDOW_HEIGHT, 0.1, 10.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_LIGHT0);
	float red[] = { 1.0, 0.1, 0.1, 1.0 };
	float white[] = { 1.0, 1.0, 1.0, 1.0 };
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, red);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, white);
	glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 60.0);

	return true;
}


// Create a Pixel Buffer Object
void createPBO(GLuint* pbo)
{
	// set up data parameter
	num_texels = image_width * image_height;
	num_values = num_texels * 4;
	size_tex_data = sizeof(GLubyte) * num_values;

	// create buffer object
	glGenBuffers(1, pbo);
	glBindBuffer(GL_ARRAY_BUFFER, *pbo);

	// buffer data
	glBufferData(GL_ARRAY_BUFFER, size_tex_data, NULL, GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void createTexture( GLuint* tex_name, unsigned int size_x, unsigned int size_y)
{
	// create a texture
	glGenTextures(1, tex_name);
	glBindTexture(GL_TEXTURE_2D, *tex_name);

	// set basic parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	// buffer data
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size_x, size_y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
}

void DisplayGL()
{
	// Render the 3D teapot with GL
	renderScene();

	// process 
	processImage();

	// flip backbuffer to screen
	displayImage();
	glutSwapBuffers();
	glutPostRedisplay();
}

void pboRegister()
{    
	// Explicit Copy 
	// map the PBO to copy data to the CL buffer via host
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, pbo_source);    

	GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_PACK_BUFFER_ARB, GL_READ_ONLY_ARB);

	cl.writeBuffer(cl_pbos[0], ptr, sizeof(unsigned int) * image_height * image_width, 0);

	glUnmapBufferARB(GL_PIXEL_PACK_BUFFER_ARB);
	glBindBufferARB(GL_PIXEL_PACK_BUFFER_ARB, 0);
}

void pboUnregister()
{
	// Explicit Copy 
	// map the PBO to copy data from the CL buffer via host
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_dest);    

	// map the buffer object into client's memory
	GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);

	cl.readBuffer(cl_pbos[1], ptr, sizeof(unsigned int) * image_height * image_width);

	glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); 
	glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}

int executeKernel(cl_int radius)
{
	// set global and local work item dimensions
	szLocalWorkSize[0] = 8;//16;
	szLocalWorkSize[1] = 8;//16;
	szGlobalWorkSize[0] = shrRoundUp((int)szLocalWorkSize[0], image_width);
	szGlobalWorkSize[1] = shrRoundUp((int)szLocalWorkSize[1], image_height);

	kernelLauncher.local(szLocalWorkSize[0], szLocalWorkSize[1]).global(szGlobalWorkSize[0], szGlobalWorkSize[1]);

	// set the args values
	cl_int tilew = (cl_int)szLocalWorkSize[0]+(2*radius);
	cl_float threshold = 0.8f;
	cl_float highlight = 4.0f;
	size_t memSize = (szLocalWorkSize[0]+(2*16))*(szLocalWorkSize[1]+(2*16))*sizeof(int);

	kernelLauncher.arg(4, tilew).arg(5, radius).arg(6, threshold).arg(7, highlight).localMemory(8, memSize);

	kernelLauncher.run();

	return 0;
}

void processImage()
{
	// activate destination buffer
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, pbo_source);

	//// read data into pbo. note: use BGRA format for optimal performance
	glReadPixels(0, 0, image_width, image_height, GL_BGRA, GL_UNSIGNED_BYTE, NULL); 

	pboRegister();
	executeKernel(blur_radius);
	pboUnregister();

	// download texture from PBO
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, pbo_dest);
	glBindTexture(GL_TEXTURE_2D, tex_screen);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_width, image_height, GL_BGRA, GL_UNSIGNED_BYTE, NULL);
}

void displayImage()
{
	// render a screen sized quad
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(-1.0, 1.0, -1.0, 1.0, -1.0, 1.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glBegin(GL_QUADS);

	glTexCoord2f(0.0, 0.0);
	glVertex3f(-1.0, -1.0, 0.5);

	glTexCoord2f(1.0, 0.0);
	glVertex3f(1.0, -1.0, 0.5);

	glTexCoord2f(1.0, 1.0);
	glVertex3f(1.0, 1.0, 0.5);

	glTexCoord2f(0.0, 1.0);
	glVertex3f(-1.0, 1.0, 0.5);

	glEnd();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glDisable(GL_TEXTURE_2D);
	glBindBuffer(GL_PIXEL_PACK_BUFFER_ARB, 0);
	glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
}

void renderScene()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60.0, (GLfloat)WINDOW_WIDTH / (GLfloat) WINDOW_HEIGHT, 0.1, 10.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, 0.0, -3.0);
	glRotatef(rotate[0], 1.0, 0.0, 0.0);
	glRotatef(rotate[1], 0.0, 1.0, 0.0);
	glRotatef(rotate[2], 0.0, 0.0, 1.0);

	glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	glutSolidTeapot(1.0);
}
void idle()
{
	rotate[0] += 0.2;
	rotate[1] += 0.6;
	rotate[2] += 1.0;

	glutPostRedisplay();
}