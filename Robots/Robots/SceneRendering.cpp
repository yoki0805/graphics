#include "SceneRendering.h"


// variables for scene entities
static GLuint listBase;
static GLuint listBullet;
static GLuint listTurretBase;
static GLuint listTurretWheels;
static GLuint listTurretBody1;
static GLuint listTurretBody2;
static GLuint listTurretBody3;
static GLuint listTurretRader;


void SetLighting(GLfloat lp[])
{
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);

	GLfloat ld[] = { 0.75, 0.75, 0.75, 1.0 };
	GLfloat ls[] = { 0.4, 0.8, 0.8, 1.0 };

	// light 0
	glLightfv(GL_LIGHT0, GL_DIFFUSE, ld);
	// light 1
	glLightfv(GL_LIGHT1, GL_DIFFUSE, ld);
	glLightfv(GL_LIGHT1, GL_SPECULAR, ls);
	glLightfv(GL_LIGHT1, GL_POSITION, lp);
}

void SetShadowMatrix(glm::mat4& mat, const glm::vec4& lpos, const glm::vec4& plane)
{
	//dot product of plane and light position
	float dot = glm::dot(plane, lpos);

	//first column
	mat[0][0] = dot - lpos.x * plane.x;
	mat[1][0] = 0 - lpos.x * plane.y;
	mat[2][0] = 0 - lpos.x * plane.z;
	mat[3][0] = 0 - lpos.x * plane.w;

	//second column
	mat[0][1] = 0 - lpos.y * plane.x;
	mat[1][1] = dot - lpos.y * plane.y;
	mat[2][1] = 0 - lpos.y * plane.z;
	mat[3][1] = 0 - lpos.y * plane.w;

	//third column
	mat[0][2] = 0 - lpos.z * plane.x;
	mat[1][2] = 0 - lpos.z * plane.y;
	mat[2][2] = dot - lpos.z * plane.z;
	mat[3][2] = 0 - lpos.z * plane.w;

	//fourth column
	mat[0][3] = 0 - lpos.w * plane.x;
	mat[1][3] = 0 - lpos.w * plane.y;
	mat[2][3] = 0 - lpos.w * plane.z;
	mat[3][3] = dot - lpos.w * plane.w;
}

void SetBaseDisplayList()
{
	listBase = glGenLists(1);
	glNewList(listBase, GL_COMPILE);

	float ka[] = { 0.1, 0.1, 0.1, 1.0 };
	float kd[] = { 0.7, 0.7, 0.7, 1.0 };
	float ks[] = { 0.3, 0.3, 0.3, 1.0 };

	glMaterialfv(GL_FRONT, GL_AMBIENT, ka);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, kd);
	glMaterialfv(GL_FRONT, GL_SPECULAR, ks);
	glMaterialf(GL_FRONT, GL_SHININESS, 45.0);

	// top face
	for (int i = -20; i < 20; ++i) {
		glBegin(GL_TRIANGLE_STRIP);
		glNormal3f(0, 1, 0);
		for (int j = -20; j <= 20; ++j) {
			glVertex3i(i + 1, 0, j);
			glVertex3i(i, 0, j);
		}
		glEnd();
	}

	// other faces
	glBegin(GL_QUADS);
	// left
	glNormal3f(-1, 0, 0);
	glVertex3i(-20, 0, -20); glVertex3i(-20, -2, -20);
	glVertex3i(-20, -2, 20); glVertex3i(-20, 0, 20);
	// right
	glNormal3f(1, 0, 0);
	glVertex3i(20, -2, -20); glVertex3i(20, 0, -20);
	glVertex3i(20, 0, 20); glVertex3i(20, -2, 20);
	// front
	glNormal3f(0, 0, 1);
	glVertex3i(-20, 0, 20); glVertex3i(-20, -2, 20);
	glVertex3i(20, -2, 20); glVertex3i(20, 0, 20);
	// back
	glNormal3f(0, 0, -1);
	glVertex3i(-20, -2, -20); glVertex3i(-20, 0, -20);
	glVertex3i(20, 0, -20); glVertex3i(20, -2, -20);
	// bottom
	glNormal3f(0, -1, 0);
	glVertex3i(-20, -2, -20); glVertex3i(20, -2, -20);
	glVertex3i(20, -2, 20); glVertex3i(-20, -2, 20);
	glEnd();

	glEndList();
}

void DrawBase(float halfSize)
{
	glPushMatrix();
	float s = 0.05 * halfSize;
	glScalef(s, 1, s);
	glColor4f(0, 0, 1, 0.65);  // set color to blue
	glCallList(listBase);
	glPopMatrix();
}

void SetBulletDisplayList()
{
	listBullet = glGenLists(1);
	glNewList(listBullet, GL_COMPILE);
	// Sphere
	glutSolidSphere(1, 10, 6);
	glEndList();
}

void DrawBullet(float halfSize)
{
	glPushMatrix();
	glEnable(GL_NORMALIZE);
	glScalef(halfSize, halfSize, halfSize);
	glColor4f(1, 1, 0, 0.65);  // set color to yellow
	glCallList(listBullet);
	glDisable(GL_NORMALIZE);
	glPopMatrix();
}

void DrawBulletShadow(float halfSize)
{
	glPushMatrix();
	glDisable(GL_LIGHTING);
	glColor4f(0, 0, 0, 0.35);  // shadow color
	glScalef(halfSize, halfSize, halfSize);
	glCallList(listBullet);
	glPopMatrix();
}

void SetTurretBaseDisplayList()
{
	listTurretBase = glGenLists(1);
	glNewList(listTurretBase, GL_COMPILE);

	//////////////////////////////////////
	//         undercarriage            //
	//////////////////////////////////////
	glPushMatrix();
	glTranslatef(0, 0.15, 0);
	glScalef(0.35, 0.045, 0.75);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.2, -0.025);
	glRotatef(-90, 1, 0, 0);
	glScalef(0.35, 0.35, 0.35);
	glutSolidTorus(0.2, 0.3, 6, 6);
	glTranslatef(0, 0, 0.18);
	glutSolidSphere(0.3, 8, 4);
	glPopMatrix();

	glEndList();
}

void SetTurretWheelsDisplayList()
{
	listTurretWheels = glGenLists(1);
	glNewList(listTurretWheels, GL_COMPILE);

	//////////////////////////////////////
	//              wheels              //
	//////////////////////////////////////
	// left wheels
	glPushMatrix();
	glTranslatef(0.3, 0.15, 0);
	glRotatef(90, 0, 1, 0);
	glScalef(0.3, 0.3, 0.6);
	glutSolidTorus(0.2, 0.3, 12, 10);
	glTranslatef(1.03, 0, 0);
	glutSolidTorus(0.2, 0.3, 12, 10);
	glTranslatef(-2.06, 0, 0);
	glutSolidTorus(0.2, 0.3, 12, 10);
	glPopMatrix();

	// right wheels
	glPushMatrix();
	glTranslatef(-0.3, 0.15, 0);
	glRotatef(90, 0, 1, 0);
	glScalef(0.3, 0.3, 0.6);
	glutSolidTorus(0.2, 0.3, 12, 10);
	glTranslatef(1.03, 0, 0);
	glutSolidTorus(0.2, 0.3, 12, 10);
	glTranslatef(-2.06, 0, 0);
	glutSolidTorus(0.2, 0.3, 12, 10);
	glPopMatrix();

	glEndList();
}

void SetTurretBodyDisplayList1()
{
	listTurretBody1 = glGenLists(1);
	glNewList(listTurretBody1, GL_COMPILE);

	//////////////////////////////////////
	//               body1              //
	//////////////////////////////////////
	glPushMatrix();
	glTranslatef(-0.05, 0.5, -0.14);
	glScalef(0.04, 0.3, 0.04);
	glutSolidCube(1);
	glPopMatrix();

	//////////////////////////////////////
	//             lights               //
	//////////////////////////////////////
	// turret lights
	glPushMatrix();
	glTranslatef(0, 0.35, 0);
	glScalef(0.1, 0.1, 1.2);
	glTranslatef(-1.9, 0, 0);
	glutSolidTorus(0.2, 0.3, 6, 6);
	glTranslatef(3.8, 0, 0);
	glutSolidTorus(0.2, 0.3, 6, 6);
	glPopMatrix();

	glEndList();
}

void SetTurretBodyDisplayList2()
{
	listTurretBody2 = glGenLists(1);
	glNewList(listTurretBody2, GL_COMPILE);

	//////////////////////////////////////
	//               body2              //
	//////////////////////////////////////
	glPushMatrix();
	glTranslatef(0, 0.35, 0);
	glScalef(0.3, 0.1, 0.3);
	glutSolidCube(1);
	glTranslatef(0, 0, 0.25);
	glutSolidCube(0.7);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.47, -0.02);
	glScalef(0.22, 0.2, 0.22);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0.15, 0.475, -0.02);
	glScalef(0.3, 0.15, 0.08);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.475, 0.285);
	glScalef(0.08, 0.08, 1.8);
	glutSolidTorus(0.2, 0.3, 6, 8);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.35, 0.175);
	glutSolidSphere(0.04, 6, 3);
	glPopMatrix();

	glEndList();
}

void SetTurretBodyDisplayList3()
{
	listTurretBody3 = glGenLists(1);
	glNewList(listTurretBody3, GL_COMPILE);

	//////////////////////////////////////
	//               body3              //
	//////////////////////////////////////
	glPushMatrix();
	glTranslatef(0.05, 0.59, -0.02);
	glScalef(0.05, 0.05, 0.15);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.475, 0.085);
	glutSolidCone(0.08, 0.2, 8, 3);
	glTranslatef(0, 0, 0.5);
	glPushMatrix();
	glScalef(1.2, 1.2, 5);
	glutSolidTorus(0.015, 0.04, 6, 8);
	glPopMatrix();
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.528, 0.15);
	glScalef(0.04, 0.04, 0.8);
	glutSolidTorus(0.2, 0.3, 6, 8);
	glTranslatef(1, -2.2, 0);
	glutSolidTorus(0.2, 0.3, 6, 8);
	glTranslatef(-2, 0, 0);
	glutSolidTorus(0.2, 0.3, 6, 8);
	glPopMatrix();

	glEndList();
}

void SetTurretRaderDisplayList()
{
	listTurretRader = glGenLists(1);
	glNewList(listTurretRader, GL_COMPILE);

	//////////////////////////////////////
	//              rader               //
	//////////////////////////////////////
	// rader
	glPushMatrix();
	glDisable(GL_CULL_FACE);
	glTranslatef(0, 0.8, 0.055);
	glRotatef(180, 0, 1, 0);
	glScalef(2.5, 1.5, 1);
	glutSolidCone(0.08, 0.08, 12, 3);
	glEnable(GL_CULL_FACE);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(0, 0.69, 0);
	glScalef(0.02, 0.25, 0.02);
	glutSolidCube(1);
	glPopMatrix();

	glEndList();
}

void SetTurretDisplayList()
{
	SetTurretBaseDisplayList();
	SetTurretWheelsDisplayList();
	SetTurretBodyDisplayList1();
	SetTurretBodyDisplayList2();
	SetTurretBodyDisplayList3();
	SetTurretRaderDisplayList();
}


void DrawTurret(GLfloat halfSize, GLfloat thetaBase, GLfloat thetaBody, GLfloat thetaRader)
{
	glPushMatrix();
	glEnable(GL_NORMALIZE);

	float s = 2.0 * halfSize;
	glScalef(s, s, s);

	// base 
	glPushMatrix();
	glRotatef(thetaBase, 0, 1, 0);
	glColor3f(0.35, 0.35, 0.35);  // set color to grey
	glCallList(listTurretBase);

	// wheels
	glColor3f(0.05, 0.05, 0.05);  // set color to black
	glCallList(listTurretWheels);

	// body
	glPushMatrix();
	glTranslatef(0, 0, -0.025);
	glRotatef(thetaBody - thetaBase, 0, 1, 0);
	glColor3f(0.75, 0.75, 0.75);  // set color to grey
	glCallList(listTurretBody1);
	glColor3ub(100, 100, 50);  // set color to army green
	glCallList(listTurretBody2);
	glColor3ub(50, 50, 0);  // set color to iron green
	glCallList(listTurretBody3);

	// rader
	glPushMatrix();
	glTranslatef(0, 0, -0.075);
	glRotatef(thetaRader - thetaBody, 0, 1, 0);
	glColor3f(0.75, 0.75, 0.75);  // set color to grey
	glCallList(listTurretRader);
	glPopMatrix();  // pop of rader
	glPopMatrix();  // pop of body
	glPopMatrix();  //pop of base

	glDisable(GL_NORMALIZE);
	glPopMatrix();
}

void DrawTurretShadow(GLfloat halfSize, GLfloat thetaBase, GLfloat thetaBody, GLfloat thetaRader)
{
	glPushMatrix();
	glDisable(GL_LIGHTING);

	float s = 2.0 * halfSize;
	glScalef(s, s, s);
	
	// shadow color
	glColor4f(0, 0, 0, 0.35);

	// base 
	glPushMatrix();
	glRotatef(thetaBase, 0, 1, 0);
	glCallList(listTurretBase);

	// wheels
	glCallList(listTurretWheels);

	// body
	glPushMatrix();
	glTranslatef(0, 0, -0.025);
	glRotatef(thetaBody - thetaBase, 0, 1, 0);
	glCallList(listTurretBody1);
	glCallList(listTurretBody2);
	glCallList(listTurretBody3);

	// rader
	glPushMatrix();
	glTranslatef(0, 0, -0.075);
	glRotatef(thetaRader - thetaBody, 0, 1, 0);
	glCallList(listTurretRader);
	glPopMatrix();  // pop of rader
	glPopMatrix();  // pop of body
	glPopMatrix();  //pop of base

	glPopMatrix();
}