#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gl/freeglut.h>
#include "Battlefield.h"


// global variables
int pause = 0;
int speedUp = 0;
int timeInterval = 16;
int width = 1280, height = 800;
int mapHalfSize = 50;

// keyboard & mouse
int mousePos[2];
int mouseBtns[3];
int keys[128];

// viewer
glm::vec3 lookAt, eyePos;
glm::vec2 viewRot;
float viewDist;

// battlefield
Battlefield battle;


void InitDemo()
{
	// random seed
	srand((unsigned)time(NULL));

	// mouse init
	mousePos[0] = 0;
	mousePos[1] = 0;
	mouseBtns[0] = GLUT_UP;
	mouseBtns[1] = GLUT_UP;
	mouseBtns[2] = GLUT_UP;

	// keyboard init
	for (int i = 0; i < 128; ++i)
		keys[i] = 0;

	// viewer init
	lookAt = glm::vec3(0, 1.5, 0);
	viewRot = glm::vec2(-15, 45);  // rotate X & Y axis
	viewDist = 90;

	battle.setInfo(mapHalfSize, 30);

	// opengl init
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClearDepth(1.0f);
	glClearStencil(0);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	// blend function
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void Display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// set viewer's position
	gluLookAt(eyePos.x, eyePos.y, eyePos.z,
		lookAt.x, lookAt.y, lookAt.z,
		0, 1, 0);

	battle.drawScene();

	glutSwapBuffers();
}

void Reshape(int w, int h)
{
	// pipeline setting
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, (double)width / (double)height, 0.1, 350);

	// fixed window size
	glutReshapeWindow(width, height);
}

void UpdateViewer(int deltaTime)
{
	// claculate the eyePos
	eyePos = glm::rotateX(glm::vec3(0, 0, 1), viewRot.x);
	eyePos = glm::rotateY(eyePos, viewRot.y);
	eyePos = lookAt + (viewDist * eyePos);

	// get the viewer's forward & left vectors 
	glm::vec3 forward = lookAt - eyePos;
	forward.y = 0;
	forward = glm::normalize(forward);
	glm::vec3 left = glm::cross(glm::vec3(0, 1, 0), forward);

	// get move speed
	float speed = 0.05f * (float)deltaTime;

	// move the viewer in 8 directions
	if (keys['w'])
	{
		if (keys['a']) {
			lookAt += (speed / 1.414f) * (forward + left);
			eyePos += (speed / 1.414f) * (forward + left);
			return;
		}
		if (keys['d']) {
			lookAt += (speed / 1.414f) * (forward - left);
			eyePos += (speed / 1.414f) * (forward - left);
			return;
		}
		lookAt += (speed * forward);
		eyePos += (speed * forward);
	}
	else if (keys['s'])
	{
		if (keys['a']) {
			lookAt += (speed / 1.414f) * (-forward + left);
			eyePos += (speed / 1.414f) * (-forward + left);
			return;
		}
		if (keys['d']) {
			lookAt += (speed / 1.414f) * (-forward - left);
			eyePos += (speed / 1.414f) * (-forward - left);
			return;
		}
		lookAt -= (speed * forward);
		eyePos -= (speed * forward);
	}
	else if (keys['a'])
	{
		lookAt += (speed * left);
		eyePos += (speed * left);
	}
	else if (keys['d'])
	{
		lookAt -= (speed * left);
		eyePos -= (speed * left);
	}
}

void FrameRateInTitle(const char* title)
{
	static int frames = 0;
	static double lastTime = 0;

	// get the elapsed time in seconds
	double currentTime = glutGet(GLUT_ELAPSED_TIME) * 0.001;
	++frames;

	// get the frame rate
	double deltaTime = currentTime - lastTime;
	if (deltaTime >= 1)
	{
		double fps = (double)frames / deltaTime;
		lastTime = currentTime;
		frames = 0;

		// display in the window title
		char line[64];
		char fpsTile[128];
		strcpy_s(line, title);
		strcat_s(line, " [FPS: %.2f]");
		sprintf_s(fpsTile, line, fps);
		glutSetWindowTitle(fpsTile);
	}
}

void Update(int dump)
{
	glutTimerFunc(timeInterval, Update, 0);
	FrameRateInTitle("Evolving Neural Networks");

	// get delta time in millisecond
	static int lastTime = 0;
	int deltaTime = glutGet(GLUT_ELAPSED_TIME) - lastTime;
	lastTime = glutGet(GLUT_ELAPSED_TIME);

	// update 
	if (pause == 0)
		battle.update();
	// update viewer
	UpdateViewer(deltaTime);

	// refreshing
	if (speedUp) {
		// high speed mod
		static int refresh = 0;
		refresh += deltaTime;
		if (refresh >= 16) {
			refresh = 0;
			glutPostRedisplay();
		}
	}
	else
		glutPostRedisplay();
}

void SpecialKeyPress(int key, int x, int y)
{
	// speed up the framerate
	if (key == GLUT_KEY_F1) {
		speedUp ^= 1;
		timeInterval = speedUp ? 1 : 16;
	}
}

void KeyPress(unsigned char key, int x, int y)
{
	if (key == 27)
		exit(0);
	if (key == ' ')
		pause ^= 1;
	// set the key down
	char k = towlower(key);
	keys[k] = 1;
}

void KeyUp(unsigned char key, int x, int y)
{
	// set the key up
	char k = towlower(key);
	keys[k] = 0;
}

void MouseButton(int btnID, int state, int x, int y)
{
	mouseBtns[btnID] = state;
	mousePos[0] = x, mousePos[1] = y;
}


void MouseMotion(int x, int y)
{
	if (mouseBtns[GLUT_LEFT_BUTTON] == GLUT_DOWN)
	{
		// rotate
		viewRot.x += (mousePos[1] - y);
		viewRot.y += (mousePos[0] - x);
		// lock the angle of X axes
		if (viewRot.x > 75)  viewRot.x = 75;
		if (viewRot.x < -75)  viewRot.x = -75;
	}
	else if (mouseBtns[GLUT_RIGHT_BUTTON] == GLUT_DOWN)	{
		// translate
		viewDist -= 0.5f * (y - mousePos[1]);
		if (viewDist < 1) viewDist = 1;
	}

	mousePos[0] = x, mousePos[1] = y;
}

int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH | GLUT_STENCIL);
	glutInitWindowSize(width, height);
	glutInitWindowPosition(0, 0);
	glutCreateWindow("Evolving Neural Networks");

	InitDemo();

	// rendering
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutTimerFunc(timeInterval, Update, 0);

	// keyboard and mouse
	glutSpecialFunc(SpecialKeyPress);
	glutKeyboardFunc(KeyPress);
	glutKeyboardUpFunc(KeyUp);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);

	glutMainLoop();
	return 0;
}