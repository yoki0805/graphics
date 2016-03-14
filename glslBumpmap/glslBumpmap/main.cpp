#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gl/glpng.h>
#include <gl/glew.h>
#include <gl/glut.h>
#include "glm.h"
#include "skydome.h"
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
vec4 lightPosition ;
int lightMoving = 1 ;
SkyDome skydome ;
GLuint myobj ;

//texture
GLuint texFloorColor, texFloorNormal ;
GLuint texWallColor, texWallNormal ;
GLuint texAlpha ;
GLuint texTurret ;

//shader
GLuint shaderBump, locTangent, locBitangent, locDigHole ;
int useBumpmap = 1 ;


void InitDemo()
{
	//camera init
	rot = vec3( 30, 0, 0 ) ;
	eye = vec3( 0, 5, 35 ) ;

	//mouse init
	m_xy = ivec2( 0, 0 ) ;
	m_btn = ivec3( GLUT_UP, GLUT_UP, GLUT_UP ) ;

	//opengl
	glClearColor( 0.5, 0.5, 0.5, 1.0 ) ;
	glClearDepth( 1.0 ) ;
	glEnable( GL_DEPTH_TEST ) ;
	glEnable( GL_CULL_FACE ) ;

	//light setting
	float la[] = { 0.25, 0.25, 0.25, 1.0 } ;
	float ld[] = { 1.0, 1.0, 1.0, 1.0 } ;
	float ls[] = { 1.0, 1.0, 1.0, 1.0 } ;
	glLightfv( GL_LIGHT0, GL_AMBIENT, la ) ;
	glLightfv( GL_LIGHT0, GL_DIFFUSE, ld ) ;
	glLightfv( GL_LIGHT0, GL_SPECULAR, ls ) ;

	//load texture
	pngSetStandardOrientation(1) ;
	texFloorColor = pngBind( "data//cobblestone.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;
	texFloorNormal = pngBind( "data//cobblestoneDOT3.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;
	texWallColor = pngBind( "data//rock.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;
	texWallNormal = pngBind( "data//rockDOT3.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;
	texAlpha = pngBind( "data//alphamask.png", PNG_BUILDMIPMAP, PNG_ALPHA, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;
	texTurret = pngBind( "data//turret.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR ) ;

	//model loading
	GLMmodel* model = glmReadOBJ( "data//portalturret.obj" ) ;
    glmUnitize( model ) ;
    glmFacetNormals( model ) ;
    glmVertexNormals( model, 90 ) ;
    myobj = glmList( model, GLM_MATERIAL | GLM_TEXTURE | GLM_SMOOTH ) ;
    glmDelete( model ) ;

	//create skydome
	skydome.Build( 50, 180, 30, 15, "data//skydome.png" ) ;

	//init shader
	extern GLuint setShaders( char*, char* ) ;
	shaderBump = setShaders( "bumpmap.vert", "bumpmap.frag" ) ;
	glUseProgram( shaderBump ) ;
	glUniform1i( glGetUniformLocation( shaderBump, "ColorMap" ), 0 ) ;
	glUniform1i( glGetUniformLocation( shaderBump, "NormalMap" ), 1 ) ;
	glUniform1i( glGetUniformLocation( shaderBump, "AlphaMap" ), 2 ) ;
	glUniform1i( glGetUniformLocation( shaderBump, "UseBump" ), 1 ) ;
	locTangent = glGetUniformLocation( shaderBump, "Tangent" ) ;
	locBitangent = glGetUniformLocation( shaderBump, "Bitangent" ) ;
	locDigHole = glGetUniformLocation( shaderBump, "DigHole" ) ;
	glUseProgram(0) ;
}


void DrawLightPosition()
{
	glPushMatrix() ;
	glTranslatef( lightPosition[0], lightPosition[1], lightPosition[2] ) ;
	glPushAttrib( GL_ENABLE_BIT | GL_CURRENT_BIT ) ;
	glDisable( GL_TEXTURE_2D ) ;
	glDisable( GL_LIGHTING ) ;
	glColor3f( 1, 1, 0 ) ;
	glutWireSphere( 0.2, 15, 7 ) ;
	glPopAttrib() ;
	glPopMatrix() ;
}


void DrawFloor()
{
	glBegin( GL_QUADS ) ;
	//same attributes for every vertex
	glVertexAttrib3f( locTangent , 1, 0, 0 ) ;
	glVertexAttrib3f( locBitangent , 0, 0, 1 ) ;
	glNormal3f( 0, 1, 0 ) ;

	glMultiTexCoord2f( GL_TEXTURE0, 0, 0 ) ; glVertex3f( -25, 0, 25 ) ;
	glMultiTexCoord2f( GL_TEXTURE0, 5, 0 ) ; glVertex3f( 25, 0, 25 ) ;
	glMultiTexCoord2f( GL_TEXTURE0, 5, 5 ) ; glVertex3f( 25, 0, -25 ) ;
	glMultiTexCoord2f( GL_TEXTURE0, 0, 5 ) ; glVertex3f( -25, 0, -25 ) ;
	glEnd() ;
}


void DrawWall()
{
	glBegin( GL_QUADS ) ;
	//same attributes for every vertex
	glVertexAttrib3f( locTangent , 1, 0, 0 ) ;
	glVertexAttrib3f( locBitangent , 0, 1, 0 ) ;
	glNormal3f( 0, 0, 1 ) ;

	glMultiTexCoord2f( GL_TEXTURE0, 0, 0 ) ; glVertex3f( -5, 0, 0 ) ;
	glMultiTexCoord2f( GL_TEXTURE0, 1, 0 ) ; glVertex3f( 5, 0, 0 ) ;
	glMultiTexCoord2f( GL_TEXTURE0, 1, 1 ) ; glVertex3f( 5, 10, 0 ) ;
	glMultiTexCoord2f( GL_TEXTURE0, 0, 1 ) ; glVertex3f( -5, 10, 0 ) ;
	glEnd() ;
}


void Display()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity() ;

	//viewer
	glTranslatef( -eye.x, -eye.y, -eye.z ) ;
	glRotatef( rot.x, 1, 0, 0 ) ;
	glRotatef( rot.y, 0, 1, 0 ) ;
	glRotatef( rot.z, 0, 0, 1 ) ;

	glEnable( GL_LIGHT0 ) ;
	glEnable( GL_LIGHTING ) ;
	glLightfv( GL_LIGHT0, GL_POSITION, value_ptr( lightPosition ) ) ;
	DrawLightPosition() ;

	glUseProgram( shaderBump ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glActiveTexture( GL_TEXTURE0 ) ;
	glBindTexture( GL_TEXTURE_2D, texFloorColor ) ;
	glActiveTexture( GL_TEXTURE1 ) ;
	glBindTexture( GL_TEXTURE_2D, texFloorNormal ) ;
	DrawFloor() ;    //draw floor

	glActiveTexture( GL_TEXTURE0 ) ;
	glBindTexture( GL_TEXTURE_2D, texWallColor ) ;
	glActiveTexture( GL_TEXTURE1 ) ;
	glBindTexture( GL_TEXTURE_2D, texWallNormal ) ;
	glActiveTexture( GL_TEXTURE2 ) ;
	glBindTexture( GL_TEXTURE_2D, texAlpha ) ;
	glPushMatrix() ;
	glTranslatef( 0, 0, -10 ) ;
	glUniform1i( locDigHole, 1 ) ;
	DrawWall() ;
	glUniform1i( locDigHole, 0 ) ;
	glTranslatef( 10, 0, 0 ) ;
	DrawWall() ;
	glTranslatef( -20, 0, 0 ) ;
	DrawWall() ;
	glPopMatrix() ;

	glPushMatrix() ;
	glTranslatef( 15, 0, -5 ) ;
	glRotatef( -90, 0, 1, 0 ) ;
	DrawWall() ;
	glTranslatef( 10, 0, 0 ) ;
	glUniform1i( locDigHole, 1 ) ;
	DrawWall() ;
	glUniform1i( locDigHole, 0 ) ;
	glTranslatef( 10, 0, 0 ) ;
	DrawWall() ;
	glPopMatrix() ;
	glUseProgram( 0 ) ;
	glActiveTexture( GL_TEXTURE0 ) ;    //restore for fixed pipeline

	//draw in fixed pipeline
	glBindTexture( GL_TEXTURE_2D, texTurret ) ;
	glPushMatrix() ;
	glTranslatef( 7.5, 3, -2.5 ) ;
	glRotatef( -45, 0, 1, 0 ) ;
	glScalef( 3, 3, 3 ) ;
	glCallList( myobj ) ;
	glPopMatrix() ;

	//skydome
	glPushMatrix() ;
	glRotatef( -120, 0, 1, 0 ) ;    //just for the good view
	skydome.Draw() ;
	glPopMatrix() ;

	glutSwapBuffers() ;
}


void Reshape( int w, int h )
{
	glViewport( 0, 0, w, h ) ;
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	gluPerspective( 45, (double)w/h, 0.1, 200 ) ;
	gw = w, gh = h ;
}


void MouseButton( int button, int state, int x, int y )
{
	//update mouse
	m_btn[ button ] = state ;
	m_xy = vec2( x, y ) ;
}


void MouseMotion( int x, int y )
{
	if( m_btn[ GLUT_LEFT_BUTTON ] == GLUT_DOWN ) {
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


void KeyPress( unsigned char key, int x, int y )
{
	switch( key ) {
		case 27 :
			exit(0) ;
			break ;
		case ' ' :
			lightMoving ^= 1 ;
			break ;
		case 'b' :
			useBumpmap ^= 1 ;
			glUseProgram( shaderBump ) ;
			glUniform1i( glGetUniformLocation( shaderBump, "UseBump" ), useBumpmap ) ;
			glUseProgram(0) ;
			break ;
	}
}


void MyTimer( int dummy )
{
	static float theta = 0 ;
	
	if( lightMoving ) {
		theta += theta<360 ? 0.35 : -360 ;
		//rose curve
		float r = 10.0*sin( glm::radians(4.0*theta) ) ;
		//polar coordinates turn to cartesian coordinates
		vec2 xz = glm::rotate( vec2( 1.0, 0.0 ), theta ) ;
		lightPosition = vec4( xz[0]*r, 5, xz[1]*r, 1 ) ;
	}

	glutTimerFunc( 16, MyTimer, 0 ) ;
	glutPostRedisplay() ;
}


int main( int argc, char** argv )
{
	glutInit( &argc, argv ) ;
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
	glutInitWindowSize( gw, gh ) ;
	glutCreateWindow( "GLSL Bump Map Demo [KeyPress: B | Space Bar]" ) ;
  
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