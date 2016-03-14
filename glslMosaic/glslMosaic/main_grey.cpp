#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gl/glew.h>
#include <gl/glpng.h>
#include <gl/glut.h>
#include "glm.h"
#pragma comment( lib, "glew32.lib" )
using namespace glm ;


//window size
int gw = 800 ;
int gh = 600 ;

//simple viewer
vec3 rot, eye ;    //viewer
ivec2 m_xy ;    //mouse position
ivec3 m_btn ;    //mouse button

//scene
GLuint obj_al, obj_turret ;
GLuint tex_floor, tex_turret ;
float theta = 0 ;
int pause = 0 ;

//fbo
GLuint fbo ;
GLuint dbo ;
GLuint tex_fbo ;

//glsl
GLuint prog_grey ;
int turn_grey = 0 ;
float fading = 1 ;

//extern functions
extern bool InitFBO( GLuint*, GLuint*, GLuint*, int, int ) ;
extern GLuint setShaders( char*, char* ) ;


GLuint LoadOBJ( const char* filename, GLuint mode )
{
	GLMmodel* model = glmReadOBJ( const_cast<char*>(filename) ) ;
    glmUnitize( model ) ;
    glmFacetNormals( model ) ;
    glmVertexNormals( model, 90 ) ;
    GLuint obj = glmList( model, mode ) ;
    glmDelete( model ) ;
	return obj ;
}


void InitDemo()
{
	//opengl
	glClearColor( 0.5, 0.5, 0.5, 1.0 ) ;
	glClearDepth( 1.0 ) ;
	glEnable( GL_DEPTH_TEST ) ;
	glEnable( GL_CULL_FACE ) ;

	//load texture
	pngSetStandardOrientation(1) ;
	tex_floor = pngBind( "data//floor.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;
	tex_turret = pngBind( "data//turret.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;

	//load obj
	obj_al = LoadOBJ( "data//al.obj", GLM_MATERIAL | GLM_SMOOTH ) ;
	obj_turret = LoadOBJ( "data//turret.obj", GLM_MATERIAL | GLM_TEXTURE | GLM_SMOOTH ) ;

	//init fbo
	InitFBO( &fbo, &tex_fbo, &dbo, 800, 600 ) ;

	//load shaders
	prog_grey = setShaders( "grey.vert", "grey.frag" ) ;
	glUseProgram( prog_grey ) ;
	glUniform1i( glGetUniformLocation( prog_grey, "tex" ), 0 ) ;
	glUniform1f( glGetUniformLocation( prog_grey, "fading" ), 1 ) ;
	glUseProgram( 0 ) ;

	//camera init
	rot = vec3( 15, 0, 0 ) ;
	eye = vec3( 0, 1, 12 ) ;

	//mouse init
	m_xy = ivec2( 0, 0 ) ;
	m_btn = ivec3( GLUT_UP, GLUT_UP, GLUT_UP ) ;
}


void DrawFloor()
{
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glBindTexture( GL_TEXTURE_2D, tex_floor ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex3i( -7.5, 0, 7.5 ) ;
	glTexCoord2f( 2, 0 ) ; glVertex3i( 7.5, 0, 7.5 ) ;
	glTexCoord2f( 2, 2 ) ; glVertex3i( 7.5, 0, -7.5 ) ;
	glTexCoord2f( 0, 2 ) ; glVertex3i( -7.5, 0, -7.5 ) ;
	glEnd() ;
	glPopAttrib() ;
}


void DrawTurret()
{
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glEnable( GL_LIGHTING ) ;
	glEnable( GL_LIGHT0 ) ;

	glBindTexture( GL_TEXTURE_2D, tex_turret ) ;
	glPushMatrix() ;
	glTranslatef( 0, 1, 0 ) ;
	glCallList( obj_turret ) ;
	glPopMatrix() ;
	glPopAttrib() ;
}


void DrawScene()
{
	DrawFloor() ;

	//turret1
	glPushMatrix() ;
	glTranslatef( 5, 0, -1 ) ;
	glRotatef( -45, 0, 1, 0 ) ;
	DrawTurret() ;
	glPopMatrix() ;

	//turret2
	glPushMatrix() ;
	glTranslatef( -5, 0, -4 ) ;
	glRotatef( 30, 0, 1, 0 ) ;
	DrawTurret() ;
	glPopMatrix() ;

	//turret3
	glPushMatrix() ;
	glTranslatef( -4, 0, 3 ) ;
	glRotatef( 120, 0, 1, 0 ) ;
	DrawTurret() ;
	glPopMatrix() ;

	//colored object
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_LIGHTING ) ;
	glEnable( GL_LIGHT0 ) ;

	//yellow cone
	float yellow[] = { 1, 1, 0, 1 } ;
	glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, yellow ) ;
	glPushMatrix() ;
	glTranslatef( 1, 0, -5 ) ;
	glRotatef( 90, -1, 0, 0 ) ;
	glutSolidCone( 0.4, 1, 15, 7 ) ;
	glPopMatrix() ;

	//green cube
	float green[] = { 0, 1, 0, 1 } ;
	glMaterialfv( GL_FRONT, GL_AMBIENT_AND_DIFFUSE, green ) ;
	glPushMatrix() ;
	glTranslatef( 4, 0.35, 4 ) ;
	glutSolidCube( 0.7 ) ;
	glPopMatrix() ;
	glPopAttrib() ;
}


void DrawAl()
{
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_LIGHTING ) ;
	glEnable( GL_LIGHT0 ) ;

	glPushMatrix() ;
	glTranslatef( 0, 1, 0 ) ;
	glCallList( obj_al ) ;
	glPopMatrix() ;
	glPopAttrib() ;
}


void Display()
{
	//////////////////////////////////////////////////////////////////////
	//fbo
	//////////////////////////////////////////////////////////////////////
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo ) ;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity() ;

	//viewer
	glTranslatef( -eye.x, -eye.y, -eye.z ) ;
	glRotatef( rot.x, 1, 0, 0 ) ;
	glRotatef( rot.y, 0, 1, 0 ) ;
	glRotatef( rot.z, 0, 0, 1 ) ;

	DrawScene() ;

	//////////////////////////////////////////////////////////////////////
	//default framebuffer
	//////////////////////////////////////////////////////////////////////
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ) ;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

	//draw 2d quad on screen
	glMatrixMode( GL_PROJECTION ) ; glPushMatrix() ; glLoadIdentity() ; gluOrtho2D( -10, 10, -10, 10 ) ;
	glMatrixMode( GL_MODELVIEW ) ; glPushMatrix() ; glLoadIdentity() ;

	glUseProgram( prog_grey ) ;
	glDepthMask( GL_FALSE ) ;
	glActiveTexture( GL_TEXTURE0 ) ;
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glBindTexture( GL_TEXTURE_2D, tex_fbo ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( -10, -10 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 10, -10 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 10, 10 ) ;
	glTexCoord2f( 0,1 ) ; glVertex2f( -10, 10 ) ;
	glEnd() ;
	glPopAttrib() ;
	
	glMatrixMode( GL_PROJECTION ) ; glPopMatrix() ;
	glMatrixMode( GL_MODELVIEW ) ; glPopMatrix() ;
	glDepthMask( GL_TRUE ) ;
	glUseProgram( 0 ) ;

	//////////////////////////////////////////////////////////////////////
	//draw model in fixed pipeline
	//////////////////////////////////////////////////////////////////////
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ) ;
	DrawScene() ;
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ) ;
	glPushMatrix() ;
	glRotatef( -theta, 0, 1, 0 ) ;
	glTranslatef( 2.5, 0, 0 ) ;
	DrawAl() ;
	glPopMatrix() ;
	
	glutSwapBuffers() ;
}


void Reshape( int w, int h )
{
	glViewport( 0, 0, w, h ) ;
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	double aspect = (double)w / (double)h ;
	gluPerspective( 45, aspect, 0.1, 100 ) ;

	//reshape fbo
	glDeleteTextures( 1, &tex_fbo ) ;
	glDeleteRenderbuffersEXT( 1, &dbo ) ;
	glDeleteFramebuffersEXT( 1, &fbo ) ;
	InitFBO( &fbo, &tex_fbo, &dbo, w, h ) ;
	gw = w, gh = h ;
}


void KeyPress( unsigned char key, int x, int y )
{
	if( key == 27 ) {
		exit(0) ;
	}

	if( key == ' ' ) {
		turn_grey ^= 1 ;
	}

	if( key == 'p' ) {
		pause ^= 1 ;
	}
}


void MouseButton( int button, int state, int x, int y )
{
	//update mouse state
	m_btn[ button ] = state ;
	m_xy = vec2( x, y ) ;
}


void MouseMotion( int x, int y )
{
	if( m_btn[ GLUT_LEFT_BUTTON ] == GLUT_DOWN )
	{
		//rotae
		rot.x += ( y - m_xy[1] ) * 0.5 ;
		rot.y += ( x - m_xy[0] ) * 0.5 ;
		//angle lock
		if( rot.x > 90 )  rot.x = 90 ;
		if( rot.x < -90 )  rot.x = -90 ;
	} else if( m_btn[ GLUT_RIGHT_BUTTON ] == GLUT_DOWN )	{
		//translate
		eye.z -= ( y - m_xy[1] ) * 0.05 ;
		if( eye.z < 1 )    eye.z = 1 ;
	}

	m_xy = vec2( x, y ) ;
	glutPostRedisplay() ;
}


void MyTimer( int dummy )
{
	if( turn_grey ) {
		fading -= 0.032 ;
		if( fading < 0 )  fading = 0 ;
	} else {
		fading += 0.032 ;
		if( fading > 1 )  fading = 1 ;
	}
	
	//set fading
	glUseProgram( prog_grey ) ;
	glUniform1f( glGetUniformLocation( prog_grey, "fading" ), fading ) ;
	glUseProgram( 0 ) ;

	//rotation
	if( !pause )  theta += 0.96 ;
	if( theta > 360 )  theta = 0 ;

	glutTimerFunc( 16, MyTimer, 0 ) ;
	glutPostRedisplay() ;
}


int main( int argc, char** argv )
{
	glutInit( &argc, argv ) ;
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
	glutInitWindowSize( 800, 600 ) ;
	glutCreateWindow( "GLSL FBO & Mosaic Demo Part1 [Key: P | Space Bar]" ) ;
  
	glewInit() ;
	InitDemo() ;

	glutDisplayFunc( Display ) ;
	glutReshapeFunc( Reshape ) ;
	glutKeyboardFunc( KeyPress ) ;
	glutMouseFunc( MouseButton ) ;
	glutMotionFunc( MouseMotion ) ;
	glutTimerFunc( 16, MyTimer, 0 ) ;

	glutMainLoop() ;
	return 0 ;
}