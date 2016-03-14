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
GLuint fbo1, fbo2 ;
GLuint dbo1, dbo2 ;
GLuint tex_fbo1, tex_fbo2 ;

//grey
GLuint prog_grey ;
int turn_grey = 0 ;
float fading = 1 ;

//mosaic
GLuint prog_mosaic ;
float radius = 50 ;
float gran = 80 ;
vec2 center ;

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
	InitFBO( &fbo1, &tex_fbo1, &dbo1, 800, 600 ) ;
	InitFBO( &fbo2, &tex_fbo2, &dbo2, 800, 600 ) ;

	//load shaders
	prog_grey = setShaders( "grey.vert", "grey.frag" ) ;
	glUseProgram( prog_grey ) ;
	glUniform1i( glGetUniformLocation( prog_grey, "tex" ), 0 ) ;
	glUniform1f( glGetUniformLocation( prog_grey, "fading" ), 1 ) ;
	glUseProgram( 0 ) ;

	prog_mosaic = setShaders( "mosaic.vert", "mosaic.frag" ) ;
	glUseProgram( prog_mosaic ) ;
	glUniform1i( glGetUniformLocation( prog_mosaic, "tex" ), 0 ) ;
	glUniform1f( glGetUniformLocation( prog_mosaic, "gran" ), gran ) ;
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


void DrawQuadOnScreen( GLuint tex )
{
	glMatrixMode( GL_PROJECTION ) ; glPushMatrix() ; glLoadIdentity() ; gluOrtho2D( -10, 10, -10, 10 ) ;
	glMatrixMode( GL_MODELVIEW ) ; glPushMatrix() ; glLoadIdentity() ;
	
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glActiveTexture( GL_TEXTURE0 ) ;
	glBindTexture( GL_TEXTURE_2D, tex ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( -10, -10 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 10, -10 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 10, 10 ) ;
	glTexCoord2f( 0,1 ) ; glVertex2f( -10, 10 ) ;
	glEnd() ;
	glPopAttrib() ;
	
	glMatrixMode( GL_PROJECTION ) ; glPopMatrix() ;
	glMatrixMode( GL_MODELVIEW ) ; glPopMatrix() ;
}


void Display()
{
	//////////////////////////////////////////////////////////////////////
	//draw scene to fbo1
	//////////////////////////////////////////////////////////////////////
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo1 ) ;
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
	//turn to grey, and draw to fbo2
	//////////////////////////////////////////////////////////////////////
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo2 ) ;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

	//draw 2d quad on screen
	glUseProgram( prog_grey ) ;
	glDepthMask( GL_FALSE ) ;
	DrawQuadOnScreen( tex_fbo1 ) ;
	glDepthMask( GL_TRUE ) ;
	glUseProgram( 0 ) ;

	//////////////////////////////////////////////////////////////////////
	//draw model to fbo2, and compute the mosaic's position
	//////////////////////////////////////////////////////////////////////
	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE ) ;
	DrawScene() ;
	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE ) ;

	glPushMatrix() ;
	glRotatef( -theta, 0, 1, 0 ) ;
	glTranslatef( 2.5, 0, 0 ) ;
	//get the position of the al's head
	double winx, winy, winz;
	double mv[16], proj[16];
	int viewport[4];
	glGetDoublev (GL_MODELVIEW_MATRIX, mv);
	glGetDoublev (GL_PROJECTION_MATRIX, proj);
	glGetIntegerv (GL_VIEWPORT, viewport);
	gluProject (0, 1.7, 0,  mv, proj, viewport, &winx, &winy, &winz);
	center.x = winx, center.y = winy ;
	//get radius
	float r = 0.4 ;
	gluProject (0, 1.7+r, 0, mv, proj, viewport, &winx, &winy, &winz);
	radius = winy - center.y ;
	if( rot.x >50 || rot.x < -30 )    radius = -2 ;  //if camera is too high or too low, shouldn't use the mosaic filter
	//draw al
	DrawAl() ;
	glPopMatrix() ;

	//////////////////////////////////////////////////////////////////////
	//paste the mosaic, and draw to glut's buffer
	//////////////////////////////////////////////////////////////////////
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ) ;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;

	//check al's is facing you
	vec3 dir = vec3( 0, 0, 1 ) ;
	dir = glm::rotate( dir, -theta, vec3( 0, 1, 0 ) ) ;
	dir = glm::rotate( dir, rot.y, vec3( 0, 1, 0 ) ) ;
	float dotvalue = dot( dir, vec3( 0, 0, 1 ) ) ;

	if( dotvalue > -0.6 ) {
		glUseProgram( prog_mosaic ) ;
		glUniform2f( glGetUniformLocation( prog_mosaic, "center" ), center.x, center.y ) ;
		glUniform1f( glGetUniformLocation( prog_mosaic, "radius" ), radius ) ;
	}
	//draw 2d quad on screen
	DrawQuadOnScreen( tex_fbo2 ) ;
	glUseProgram( 0 ) ;
	
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
	glDeleteTextures( 1, &tex_fbo1 ) ;
	glDeleteRenderbuffersEXT( 1, &dbo1 ) ;
	glDeleteFramebuffersEXT( 1, &fbo1 ) ;
	InitFBO( &fbo1, &tex_fbo1, &dbo1, w, h ) ;

	glDeleteTextures( 1, &tex_fbo2 ) ;
	glDeleteRenderbuffersEXT( 1, &dbo2 ) ;
	glDeleteFramebuffersEXT( 1, &fbo2 ) ;
	InitFBO( &fbo2, &tex_fbo2, &dbo2, w, h ) ;
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
	glutCreateWindow( "GLSL FBO & Mosaic Demo Final [Key: P | Space Bar]" ) ;
  
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