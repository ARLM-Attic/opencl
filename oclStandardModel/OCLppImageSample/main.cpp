#include <iostream>
#include <cassert>
#include <oclUtils.h>

#include "OpenCL.h"
#include "Timer.h"

using namespace std;


cl_int ciErrNum;
ocl::OpenCL cl;
ocl::Image3D* d_volumeArray;
ocl::Image2D* d_transferFuncArray;
ocl::Buffer* d_invViewMatrix;
size_t volumeSize[3] = {32, 32, 32};
void initCLVolume(unsigned char *h_volume);

GLuint pbo = 0;                 // OpenGL pixel buffer object
ocl::Buffer* pbo_cl;
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

float density = 0.05f;
float brightness = 1.0f;
float transferOffset = 0.0f;
float transferScale = 1.0f;
bool linearFiltering = true;

void initPixelBuffer();
void render();
void initCLVolume(unsigned char *h_volume);

// OpenGL functionality
void InitGL(int argc, const char** argv);
void DisplayGL();
void KeyboardGL(unsigned char key, int x, int y);
void Reshape(int w, int h);
void motion(int x, int y);
void mouse(int button, int state, int x, int y);
void Idle(void);

ocl::Launcher kernelLauncher;
char* cPathAndName = NULL;          // var for full paths to data, src, etc.
const char *volumeFilename = "Bucky.raw";
int ox, oy;                         // mouse location vars
int buttonState = 0;       

// Helpers
void Cleanup(int iExitCode);
void (*pCleanup)(int) = &Cleanup;

bool imgSupport;

int main(int argc, const char** argv)
{
	vector<string> files;
	files.push_back("volumeRender.cl");

	ocl::Program* prog = cl.createProgram(files);
	string buildOpts = " -DIMAGE_SUPPORT" ;
	prog->build(buildOpts);

	kernelLauncher = prog->createLauncher("d_render");

	void* info = cl.getDeviceInfo(CL_DEVICE_IMAGE_SUPPORT);
	if(info != NULL) {
		imgSupport = *(bool*) info;
	}

	InitGL(argc, argv);

	cPathAndName = shrFindFilePath(volumeFilename, argv[0]);
	oclCheckErrorEX(cPathAndName != NULL, shrTRUE, pCleanup);
	size_t size = volumeSize[0] * volumeSize[1] * volumeSize[2];
	unsigned char* h_volume = shrLoadRawFile(cPathAndName, size);
	initCLVolume(h_volume);
	initPixelBuffer();
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
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutReshapeFunc(Reshape);

	glewInit();
	/*glutKeyboardFunc(KeyboardGL);

	// initialize necessary OpenGL extensions
	glewInit();
	GLboolean bGLEW = glewIsSupported("GL_VERSION_2_0 GL_ARB_pixel_buffer_object"); 
	oclCheckErrorEX(bGLEW, shrTRUE, pCleanup); */
}

void initCLVolume(unsigned char *h_volume)
{
	ciErrNum = CL_SUCCESS;

	if (imgSupport) {
		// create 3D array and copy data to device
		cl_image_format volume_format;
		volume_format.image_channel_order = CL_R;
		volume_format.image_channel_data_type = CL_UNORM_INT8;

		d_volumeArray = cl.createImage3D(volumeSize[0], volumeSize[1], volumeSize[2], volumeSize[0], 
						volumeSize[0]*volumeSize[1], CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &volume_format, h_volume);

		// create transfer function texture
		float transferFunc[] = {
			0.0, 0.0, 0.0, 0.0, 
			1.0, 0.0, 0.0, 1.0, 
			1.0, 0.5, 0.0, 1.0, 
			1.0, 1.0, 0.0, 1.0, 
			0.0, 1.0, 0.0, 1.0, 
			0.0, 1.0, 1.0, 1.0, 
			0.0, 0.0, 1.0, 1.0, 
			1.0, 0.0, 1.0, 1.0, 
			0.0, 0.0, 0.0, 0.0, 
		};

		cl_image_format transferFunc_format;
		transferFunc_format.image_channel_order = CL_RGBA;
		transferFunc_format.image_channel_data_type = CL_FLOAT;

		d_transferFuncArray = cl.createImage2D(9,1, sizeof(float) * 9 * 4, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, &transferFunc_format, transferFunc);

		kernelLauncher.arg(8, d_volumeArray->getMem()).arg(9, d_transferFuncArray->getMem());
	}

	// init invViewMatrix
	d_invViewMatrix = cl.createBuffer(12 * sizeof(float), CL_MEM_READ_ONLY);
	
	kernelLauncher.arg(7, d_invViewMatrix->getMem());
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

	kernelLauncher.arg(0, pbo_cl->getMem()).arg(1, width).arg(2, height);
}

// render image using OpenCL
//*****************************************************************************
void render()
{
    ciErrNum = CL_SUCCESS;

	d_invViewMatrix->write(invViewMatrix, 12*sizeof(float), 0, CL_FALSE);

    // execute OpenCL kernel, writing results to PBO
    size_t localSize[] = {LOCAL_SIZE_X,LOCAL_SIZE_Y};

	kernelLauncher.arg(3, density).arg(4, brightness).arg(5, transferOffset).arg(6,transferScale);
	kernelLauncher.local(localSize[0], localSize[1]).global(gridSize[0], gridSize[1]);
	kernelLauncher.run();

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
    switch(key) 
    {
        case '=':
            density += 0.01;
            break;
        case '-':
            density -= 0.01;
            break;
        case '+':
            density += 0.1;
            break;
        case '_':
            density -= 0.1;
            break;

        case ']':
            brightness += 0.1;
            break;
        case '[':
            brightness -= 0.1;
            break;

        case ';':
            transferOffset += 0.01;
            break;
        case '\'':
            transferOffset -= 0.01;
            break;

        case '.':
            transferScale += 0.01;
            break;
        case ',':
            transferScale -= 0.01;
            break;
        case '\033': // escape quits
        case '\015': // Enter quits    
        case 'Q':    // Q quits
        case 'q':    // q (or escape) quits
            // Cleanup up and quit
            Cleanup(EXIT_SUCCESS);
            break;
        default:
            break;
    }
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