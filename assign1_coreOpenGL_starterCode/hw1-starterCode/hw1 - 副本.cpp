/*
  CSCI 420 Computer Graphics, USC
  Assignment 1: Height Fields with Shaders.
  C++ starter code

  Student username: <type your USC username here>
*/

#include "basicPipelineProgram.h"
#include "openGLMatrix.h"
#include "imageIO.h"
#include "openGLHeader.h"
#include "glutHeader.h"

#include <iostream>
#include <cstring>

#if defined(WIN32) || defined(_WIN32)
#ifdef _DEBUG
#pragma comment(lib, "glew32d.lib")
#else
#pragma comment(lib, "glew32.lib")
#endif
#endif

#if defined(WIN32) || defined(_WIN32)
char shaderBasePath[1024] = SHADER_BASE_PATH;
#else
char shaderBasePath[1024] = "../openGLHelper-starterCode";
#endif

using namespace std;

int mousePos[2]; // x,y coordinate of the mouse position

int leftMouseButton = 0; // 1 if pressed, 0 if not 
int middleMouseButton = 0; // 1 if pressed, 0 if not
int rightMouseButton = 0; // 1 if pressed, 0 if not

int mode = 1;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROL_STATE;
CONTROL_STATE controlState = ROTATE;

// state of the world
float landRotate[3] = { 0.0f, 0.0f, 0.0f };
float landTranslate[3] = { 0.0f, 0.0f, 0.0f };
float landScale[3] = { 1.0f, 1.0f, 1.0f };

int windowWidth = 1280;
int windowHeight = 720;
char windowTitle[512] = "CSCI 420 homework I";

ImageIO* heightmapImage;

//Create VBO
GLuint PointVertexBuffer, PointColorVertexBuffer;
GLuint LineVertexBuffer, LineColorVertexBuffer;
GLuint TriVertexBuffer, TriColorVertexBuffer;
//Create VAO
GLuint PointVertexArray;
GLuint LineVertexArray;
GLuint TriVertexArray;
int sizePoint;
int sizeLine;
int sizeTri;

OpenGLMatrix matrix;
BasicPipelineProgram* pipelineProgram;

// write a screenshot to the specified filename
void saveScreenshot(const char* filename)
{
	unsigned char* screenshotData = new unsigned char[windowWidth * windowHeight * 3];
	glReadPixels(0, 0, windowWidth, windowHeight, GL_RGB, GL_UNSIGNED_BYTE, screenshotData);

	ImageIO screenshotImg(windowWidth, windowHeight, 3, screenshotData);

	if (screenshotImg.save(filename, ImageIO::FORMAT_JPEG) == ImageIO::OK)
		cout << "File " << filename << " saved successfully." << endl;
	else cout << "Failed to save file " << filename << '.' << endl;

	delete[] screenshotData;
}

void displayFunc()
{
	// render some stuff...
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	matrix.SetMatrixMode(OpenGLMatrix::ModelView);//change the mode to ModelView
	matrix.LoadIdentity();
	matrix.LookAt(0, 0, 5, 0, 0, 0, 0, 1, 0);

	//ModelView
	matrix.Translate(landTranslate[0], landTranslate[1], landTranslate[2]);
	matrix.Rotate(landRotate[0], 1.0f, 0.0f, 0.0f);
	matrix.Rotate(landRotate[1], 0.0f, 1.0f, 0.0f);
	matrix.Rotate(landRotate[2], 0.0f, 0.0f, 1.0f);
	matrix.Scale(landScale[0], landScale[1], landScale[2]);

	float m[16];
	matrix.SetMatrixMode(OpenGLMatrix::ModelView);
	matrix.GetMatrix(m);

	float p[16];
	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.GetMatrix(p);

	// bind shader
	pipelineProgram->Bind();

	// set variable
	pipelineProgram->SetModelViewMatrix(m);//upload ModelView Matrix to GPU
	pipelineProgram->SetProjectionMatrix(p);//upload Projection Matrix to GPU

	if (mode == 1) {
		glBindVertexArray(PointVertexArray);
		glDrawArrays(GL_POINTS, 0, sizePoint);//causes sizeTri data to be rendered starting with the first point
		//cout << "mode 1" << endl;
	}
	else if (mode == 2) {
		glBindVertexArray(LineVertexArray);
		glDrawArrays(GL_LINES, 0, sizeLine);
		//cout << "mode 2" << endl;
	}
	else if (mode == 3) {
		glBindVertexArray(TriVertexArray);
		glDrawArrays(GL_TRIANGLES, 0, sizeTri);
		//cout << "mode 3" << endl;
	}

	glutSwapBuffers();
}

void idleFunc()
{
	// do some stuff... 

	// for example, here, you can save the screenshots to disk (to make the animation)

	// make the screen update 
	glutPostRedisplay();
}

void reshapeFunc(int w, int h)
{
	glViewport(0, 0, w, h);

	matrix.SetMatrixMode(OpenGLMatrix::Projection);
	matrix.LoadIdentity();
	matrix.Perspective(54.0f, (float)w / (float)h, 0.01f, 100.0f);
}

void mouseMotionDragFunc(int x, int y)
{
	// mouse has moved and one of the mouse buttons is pressed (dragging)

	// the change in mouse position since the last invocation of this function
	int mousePosDelta[2] = { x - mousePos[0], y - mousePos[1] };

	switch (controlState)
	{
		// translate the landscape
	case TRANSLATE://go in when ctrl+dragging
		if (leftMouseButton)
		{
			// control x,y translation via the left mouse button
			landTranslate[0] += mousePosDelta[0] * 0.01f;
			landTranslate[1] -= mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z translation via the middle mouse button
			landTranslate[2] += mousePosDelta[1] * 0.01f;
		}
		break;

		// rotate the landscape
	case ROTATE://go in when both Ctrl & SHIFT are not pressed
		if (leftMouseButton)
		{
			// control x,y rotation via the left mouse button
			landRotate[0] += mousePosDelta[1];
			landRotate[1] += mousePosDelta[0];
		}
		if (middleMouseButton)
		{
			// control z rotation via the middle mouse button
			landRotate[2] += mousePosDelta[1];
		}
		break;

		// scale the landscape
	case SCALE:
		if (leftMouseButton)
		{
			// control x,y scaling via the left mouse button
			landScale[0] *= 1.0f + mousePosDelta[0] * 0.01f;
			landScale[1] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		if (middleMouseButton)
		{
			// control z scaling via the middle mouse button
			landScale[2] *= 1.0f - mousePosDelta[1] * 0.01f;
		}
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseMotionFunc(int x, int y)
{
	// mouse has moved
	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void mouseButtonFunc(int button, int state, int x, int y)
{
	// a mouse button has has been pressed or depressed

	// keep track of the mouse button state, in leftMouseButton, middleMouseButton, rightMouseButton variables
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		leftMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_MIDDLE_BUTTON:
		middleMouseButton = (state == GLUT_DOWN);
		break;

	case GLUT_RIGHT_BUTTON:
		rightMouseButton = (state == GLUT_DOWN);
		break;
	}

	// keep track of whether CTRL and SHIFT keys are pressed
	switch (glutGetModifiers())
	{
	case GLUT_ACTIVE_CTRL:
		controlState = TRANSLATE;
		break;

	case GLUT_ACTIVE_SHIFT:
		controlState = SCALE;
		break;

		// if CTRL and SHIFT are not pressed, we are in rotate mode
	default:
		controlState = ROTATE;
		break;
	}

	// store the new mouse position
	mousePos[0] = x;
	mousePos[1] = y;
}

void keyboardFunc(unsigned char key, int x, int y)
{
	switch (key)
	{
	case 27: // ESC key
		exit(0); // exit the program
		break;

	case ' ':
		cout << "You pressed the spacebar." << endl;
		break;

	case 'x':
		// take a screenshot
		saveScreenshot("screenshot.jpg");
		break;

	case '1': {
		mode = 1;
		break;
	}
	case '2': {
		mode = 2;
		break;
	}
	case '3': {
		mode = 3;
		break;
	}
			//case 'w':
	}
}

void initScene(int argc, char* argv[])
{

	// load the image from a jpeg disk file to main memory
	heightmapImage = new ImageIO();
	if (heightmapImage->loadJPEG(argv[1]) != ImageIO::OK)
	{
		cout << "Error reading image " << argv[1] << "." << endl;
		exit(EXIT_FAILURE);
	}

	int map_height = heightmapImage->getHeight();
	int map_width = heightmapImage->getWidth();

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//mode1
	glm::vec3* points = new glm::vec3[map_height * map_width];
	glm::vec4* color_points = new glm::vec4[map_height * map_width];
	for (int i = 0; i < map_height; i++) {
		for (int j = 0; j < map_width; j++) {
			points[i * map_width + j] = glm::vec3((float)(i - map_height / 2) / map_height * 4, (float)(j - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[i * map_width + j]) / 256);
			color_points[i * map_width + j] = { (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256 ,(float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256,(float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, 1 };
		}
	}

	glGenBuffers(1, &PointVertexBuffer);//gllGenBuffers returns 1 buffer object names (unsigned integers) in triVertexBuffer 
										//if set to 0, then unbinds any buffer object ppreviously bound
	glBindBuffer(GL_ARRAY_BUFFER, PointVertexBuffer);//glBindBuffer binds the buffer object name(triVertexBuffer) to the target (GL_ARRAY_BUFFER->vertex attributes)， indicates that the data in the buffer will be vertex attribute data rather than some one of the other storage types
													//A target can only bond with one buffer object. So when a buffer object is bound to a target, the previous binding for that target is automatically broken.
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * map_height * map_width, points,//create a new data store (allocate) in GPU for the buffer object currently bound to target(GL_ARRAY_BUFFER) (which is PointVertexBuffer in this case) 
		GL_STATIC_DRAW);

	glGenBuffers(1, &PointColorVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, PointColorVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * map_height * map_width, color_points, GL_STATIC_DRAW);

	pipelineProgram = new BasicPipelineProgram;
	int ret = pipelineProgram->Init(shaderBasePath);
	if (ret != 0) abort();

	glGenVertexArrays(1, &PointVertexArray);//glGenVertexArray returns 1 vertex array object names in triVertexArray (avoid reuse of names)
	glBindVertexArray(PointVertexArray);//glBindVertexArray binds the vertex array object with name triVertexArray
	
	glBindBuffer(GL_ARRAY_BUFFER, PointVertexBuffer);
	GLuint loc =
		glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");//glGetAttribuLocation returns the index of an attribute variable ("position" in vertex shader in this example)
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);//2nd,3rd parameters: the array points is a 3-d array of floats																			//4th parameter: we do not want the data normalized to be the range (0.0, 1.0);
																			//5th parameter: the value in the array are contiguous
	glBindBuffer(GL_ARRAY_BUFFER, PointColorVertexBuffer);
	loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	glEnable(GL_DEPTH_TEST);

	sizePoint = map_width * map_height;

	std::cout << "GL error: " << glGetError() << std::endl;

	//mode2
	int test = map_height * (map_width - 2) * 2 + map_height * map_width * 2;
	glm::vec3* line = new glm::vec3[test];
	glm::vec4* color_line = new glm::vec4[test];

	for (int i = 0; i < map_height; i++) {
		for (int j = 0; j < map_width; j++) {
			line[i * map_height + j] = glm::vec3((float)(i - map_height / 2) / map_height * 4, (float)(j - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[i * map_width + j]) / 256);
			line[map_height * map_width + i * map_height + j] = glm::vec3((float)(j - map_height / 2) / map_height * 4, (float)(i - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[j * map_width + i]) / 256);
			color_line[i * map_height + j] = { (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, 1 };
			color_line[map_height * map_width + i * map_height + j] = { (float)((int)(heightmapImage->getPixels()[j * map_width + i])) / 256, (float)((int)(heightmapImage->getPixels()[j * map_width + i])) / 256, (float)((int)(heightmapImage->getPixels()[j * map_width + i])) / 256, 1 };
		}
	}


	for (int i = 0; i < map_height; i++) {
		for (int j = 1; j < map_width - 1; j++) {
			line[map_height * map_width * 2 + i * (map_width - 2) + j - 1] = glm::vec3((float)(i - map_height / 2) / map_height * 4, (float)(j - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[i * map_width + j]) / 256);
			line[map_height * map_width * 2 + (map_width - 2) * map_height + i * (map_height - 2) + j - 1] = glm::vec3((float)(j - map_height / 2) / map_height * 4, (float)(i - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[j * map_width + i]) / 256);
			color_line[map_height * map_width * 2 + (map_width - 2) * map_height + i * (map_height - 2) + j - 1] = { (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, 1 };
			color_line[map_height * map_width * 2 + i * (map_width - 2) + j - 1] = { (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, 1 };

		}
	}

	glGenBuffers(1, &LineVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, LineVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * test, line,
		GL_STATIC_DRAW);

	glGenBuffers(1, &LineColorVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, LineColorVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * test, color_line, GL_STATIC_DRAW);

	pipelineProgram = new BasicPipelineProgram;
	ret = pipelineProgram->Init(shaderBasePath);
	//cout <<"test:" << ret << endl;
	if (ret != 0) abort();

	glGenVertexArrays(1, &LineVertexArray);
	glBindVertexArray(LineVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, LineVertexBuffer);

	loc =
		glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, LineColorVertexBuffer);
	loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	glEnable(GL_DEPTH_TEST);

	sizeLine = test;

	std::cout << "GL error: " << glGetError() << std::endl;

	//mode3
	int test_3 = (map_width - 1) * (map_height - 1) * 6;
	glm::vec3* triangle = new glm::vec3[test_3];
	glm::vec4* color_tri = new glm::vec4[test_3];

	for (int i = 0; i < map_height - 1; i++) {
		for (int j = 0; j < map_width - 1; j++) {
			triangle[6 * i * (map_width - 1) + 6 * j] = triangle[6 * i * (map_width - 1) + 6 * j + 3] = glm::vec3((float)(i - map_height / 2) / map_height * 4, (float)(j - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[i * map_width + j]) / 256);
			triangle[6 * i * (map_width - 1) + 6 * j + 1] = glm::vec3((float)(i - map_height / 2) / map_height * 4, (float)((j + 1) - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[i * map_width + (j + 1)]) / 256);
			triangle[6 * i * (map_width - 1) + 6 * j + 2] = triangle[6 * i * (map_width - 1) + 6 * j + 4] = glm::vec3((float)((i + 1) - map_height / 2) / map_height * 4, (float)((j + 1) - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[(i + 1) * map_width + (j + 1)]) / 256);
			triangle[6 * i * (map_width - 1) + 6 * j + 5] = glm::vec3((float)((i + 1) - map_height / 2) / map_height * 4, (float)(j - map_width / 2) / map_width * 4, (float)(int)(heightmapImage->getPixels()[(i + 1) * map_width + j]) / 256);
			color_tri[6 * i * (map_width - 1) + 6 * j] = color_tri[6 * i * (map_width - 1) + 6 * j + 3] = { (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + j])) / 256, 1 };
			color_tri[6 * i * (map_width - 1) + 6 * j + 1] = { (float)((int)(heightmapImage->getPixels()[i * map_width + (j + 1)])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + (j + 1)])) / 256, (float)((int)(heightmapImage->getPixels()[i * map_width + (j + 1)])) / 256, 1 };
			color_tri[6 * i * (map_width - 1) + 6 * j + 2] = color_tri[6 * i * (map_width - 1) + 6 * j + 4] = { (float)((int)(heightmapImage->getPixels()[(i + 1) * map_width + (j + 1)])) / 256, (float)((int)(heightmapImage->getPixels()[(i + 1) * map_width + (j + 1)])) / 256, (float)((int)(heightmapImage->getPixels()[(i + 1) * map_width + (j + 1)])) / 256, 1 };
			color_tri[6 * i * (map_width - 1) + 6 * j + 5] = { (float)((int)(heightmapImage->getPixels()[(i + 1) * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[(i + 1) * map_width + j])) / 256, (float)((int)(heightmapImage->getPixels()[(i + 1) * map_width + j])) / 256, 1 };
		}
	}

	glGenBuffers(1, &TriVertexBuffer);//TriVertexBuffer = 5
	glBindBuffer(GL_ARRAY_BUFFER, TriVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * test_3, triangle,
		GL_STATIC_DRAW);

	glGenBuffers(1, &TriColorVertexBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, TriColorVertexBuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec4) * test_3, color_tri, GL_STATIC_DRAW);

	pipelineProgram = new BasicPipelineProgram;
	ret = pipelineProgram->Init(shaderBasePath);//ret = 0
	if (ret != 0) abort();

	glGenVertexArrays(1, &TriVertexArray);
    glBindVertexArray(TriVertexArray);
	glBindBuffer(GL_ARRAY_BUFFER, TriVertexBuffer);

	loc =
		glGetAttribLocation(pipelineProgram->GetProgramHandle(), "position");//loc = 0
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);

	glBindBuffer(GL_ARRAY_BUFFER, TriColorVertexBuffer);
	loc = glGetAttribLocation(pipelineProgram->GetProgramHandle(), "color");//loc = 1
	glEnableVertexAttribArray(loc);
	glVertexAttribPointer(loc, 4, GL_FLOAT, GL_FALSE, 0, (const void*)0);//original;:4

	glEnable(GL_DEPTH_TEST);

	sizeTri = test_3;

	std::cout << "GL error: " << glGetError() << std::endl;

	//cout << "loc1: " << loc << " loc_2: " << loc_2 << " loc_3: " << loc_3 << endl;
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		cout << "The arguments are incorrect." << endl;
		cout << "usage: ./hw1 <heightmap file>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Initializing GLUT..." << endl;
	glutInit(&argc, argv);

	cout << "Initializing OpenGL..." << endl;

#ifdef __APPLE__
	glutInitDisplayMode(GLUT_3_2_CORE_PROFILE | GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#else
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
#endif

	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(windowTitle);

	cout << "OpenGL Version: " << glGetString(GL_VERSION) << endl;
	cout << "OpenGL Renderer: " << glGetString(GL_RENDERER) << endl;
	cout << "Shading Language Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;

#ifdef __APPLE__
	// This is needed on recent Mac OS X versions to correctly display the window.
	glutReshapeWindow(windowWidth - 1, windowHeight - 1);
#endif

	// tells glut to use a particular display function to redraw 
	glutDisplayFunc(displayFunc);
	// perform animation inside idleFunc
	glutIdleFunc(idleFunc);
	// callback for mouse drags
	glutMotionFunc(mouseMotionDragFunc);
	// callback for idle mouse movement
	glutPassiveMotionFunc(mouseMotionFunc);
	// callback for mouse button changes
	glutMouseFunc(mouseButtonFunc);
	// callback for resizing the window
	glutReshapeFunc(reshapeFunc);
	// callback for pressing the keys on the keyboard
	glutKeyboardFunc(keyboardFunc);

	// init glew
#ifdef __APPLE__
  // nothing is needed on Apple
#else
  // Windows, Linux
	GLint result = glewInit();
	if (result != GLEW_OK)
	{
		cout << "error: " << glewGetErrorString(result) << endl;
		exit(EXIT_FAILURE);
	}
#endif

	// do initialization
	initScene(argc, argv);

	// sink forever into the glut loop
	glutMainLoop();
}


