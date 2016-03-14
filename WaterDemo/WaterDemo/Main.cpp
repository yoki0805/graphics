#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/glut.h>
#include <GL/glpng.h>
#pragma comment( lib, "glpngd.lib" )

using namespace std ;
using namespace glm ;


//global variables
const int WaterGridSize = 150 ;
vec3 WaterSurface[ WaterGridSize ][ WaterGridSize ] ;
vec3 WaterNormal[ WaterGridSize ][ WaterGridSize ] ;
vec2 WaterTexCoord[ WaterGridSize ][ WaterGridSize ] ;
GLfloat WaterVelocity[ WaterGridSize ][ WaterGridSize ] ;
GLuint WaterTexture ;



void InitWater()
{
	float waterDim = 3.0f / (float)(WaterGridSize-1) ;

	for( int i = 0 ; i < WaterGridSize ; ++i )
		for( int j = 0 ; j < WaterGridSize ; ++j )
		{
			WaterSurface[ i ][ j ].x = i - WaterGridSize/2 ;
			WaterSurface[ i ][ j ].y = 0.0f ;
			WaterSurface[ i ][ j ].z = j - WaterGridSize/2 ;

			WaterTexCoord[ i ][ j ].s = i * waterDim ;
			WaterTexCoord[ i ][ j ].t = 1.0f - ( j * waterDim ) ;

			WaterNormal[ i ][ j ] = vec3( 0.0f, 1.0f, 0.0f ) ;
			WaterVelocity[ i ][ j ] = 0.0f ;
		}
}


void InitDemo()
{
	//opengl Init
	glClearColor( 0.0, 0.0, 0.0, 1.0 ) ;
	glClearDepth( 1.0 ) ;
	
    glEnable( GL_DEPTH_TEST ) ;
	glEnable( GL_CULL_FACE ) ;
	glDepthFunc( GL_LEQUAL ) ;
	glShadeModel( GL_SMOOTH ) ;

	InitWater() ;
	WaterTexture = pngBind( "data/water1.png", PNG_NOMIPMAP, PNG_SOLID, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
}


void DrawWater()
{
	static float offset = 0.0f ;

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT ) ;

	float s, t ;
	for( int i = 0 ; i < WaterGridSize-1 ; ++i )
	{
		glBegin( GL_TRIANGLE_STRIP ) ;
		for( int j = 0 ; j < WaterGridSize ; ++j )
		{
			s = WaterTexCoord[ i+1 ][ j ].s + WaterNormal[ i+1 ][ j ].x + offset ;
			t = WaterTexCoord[ i+1 ][ j ].t + WaterNormal[ i+1 ][ j ].z + offset ;

			glTexCoord2f( s, t ) ;
			glNormal3fv( value_ptr( WaterNormal[ i+1 ][ j ] ) ) ;
			glVertex3fv( value_ptr( WaterSurface[ i+1 ][ j ] ) ) ;

			s = WaterTexCoord[ i ][ j ].s + WaterNormal[ i ][ j ].x + offset ;
			t = WaterTexCoord[ i ][ j ].t + WaterNormal[ i ][ j ].z + offset ;

			glTexCoord2f( s, t ) ;
			glNormal3fv( value_ptr( WaterNormal[ i ][ j ] ) ) ;
			glVertex3fv( value_ptr( WaterSurface[ i ][ j ] ) ) ;
		}
		glEnd() ;
	}

	offset -= 0.00075f ;
}


void CalcWater()
{
	float deltah ;
	vec3 temp ;

	for( int i = 1 ; i < WaterGridSize-1 ; ++i )
		for( int j = 1 ; j < WaterGridSize-1 ; ++j )
		{
			deltah = WaterSurface[ i ][ j ].y - ( WaterSurface[ i+1 ][ j ].y + WaterSurface[ i-1 ][ j ].y + WaterSurface[ i ][ j+1 ].y + WaterSurface[ i ][ j-1 ].y ) * 0.25f ;
			WaterVelocity[ i ][ j ] += deltah ;
		}

	for( int i = 1 ; i < WaterGridSize-1 ; ++i )
		for( int j = 1 ; j < WaterGridSize-1 ; ++j )
		{
			WaterSurface[ i ][ j ].y -= WaterVelocity[ i ][ j ] ;
			WaterVelocity[ i ][ j ] *= 0.97f ;
		}

	for( int i = 1 ; i < WaterGridSize-1 ; ++i )
		for( int j = 1 ; j < WaterGridSize-1 ; ++j )
		{
			temp.x = WaterSurface[ i+1 ][ j ].y - WaterSurface[ i-1 ][ j ].y ;
			temp.y = 1.0f ;
			temp.z = WaterSurface[ i ][ j+1 ].y - WaterSurface[ i ][ j-1 ].y ;

			WaterNormal[ i ][ j ] = normalize( temp ) ;
		}
}


void Display()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity() ;

	gluLookAt( 0, 30, 90,
						  0, 0, 0,
						  0, 1, -1 ) ;

	//glPolygonMode( GL_FRONT_AND_BACK, GL_LINE ) ;
	
	/*
	glEnable( GL_TEXTURE_GEN_S ) ;
	glEnable( GL_TEXTURE_GEN_T ) ;

	glTexGeni( GL_S, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
	glTexGeni( GL_T, GL_TEXTURE_GEN_MODE, GL_SPHERE_MAP ) ;
	*/

	glEnable( GL_TEXTURE_2D ) ;
	glBindTexture( GL_TEXTURE_2D, WaterTexture ) ;
	DrawWater() ;	

	glutSwapBuffers() ;
}


void Reshape( GLsizei width, GLsizei height )
{
	if( height == 0 )    height = 1 ;
	glViewport( 0, 0, width, height ) ;

	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	gluPerspective( 45, (GLdouble)width / (GLdouble)height, 0.1, 500 ) ;

	glutPostRedisplay() ;
}


void Screen2World( int x, int y, float* xz )
{
	GLint viewport[ 4 ] ;
	GLdouble mvmatrix[ 16 ], projmatrix[ 16 ] ;
	GLint realy ;    //OpenGL y coordinate position
	GLdouble wx0, wy0, wz0, wx1, wy1, wz1 ;    //returned world x, y, z coords
    GLdouble t ;

	glGetIntegerv( GL_VIEWPORT, viewport ) ;
    glGetDoublev( GL_MODELVIEW_MATRIX, mvmatrix ) ;
    glGetDoublev( GL_PROJECTION_MATRIX, projmatrix ) ;

    //note viewport[3] is height of window in pixels
    realy = viewport[3] - (GLint)y - 1 ;
		
    gluUnProject( (GLdouble)x, (GLdouble)realy, 0.0, 
            mvmatrix, projmatrix, viewport, &wx0, &wy0, &wz0 ) ; 

    gluUnProject( (GLdouble)x, (GLdouble)realy, 1.0, 
            mvmatrix, projmatrix, viewport, &wx1, &wy1, &wz1 ) ; 

	//find intersection at XZ plane (y=0)
    t = wy0 / (wy0-wy1) ;
	xz[0] = (1-t) * wx0 + t * wx1 ;
	xz[2] = (1-t) * wz0 + t * wz1 ;
    xz[1] = 0.0 ;
}


void Update( int value )
{
	glutTimerFunc( 16, Update, 0 ) ;
	CalcWater() ;
	glutPostRedisplay() ;
}


void MouseButton( int button, int state, int x, int y )
{

}


void MousePassive( int x, int y )
{
	vec3 pos ;
	Screen2World( x, y, value_ptr( pos ) ) ;

	float len ;
	for( int i = 0 ; i < WaterGridSize ; ++i )
		for( int j = 0 ; j < WaterGridSize ; ++j )
		{
			len = ( pos.x - WaterSurface[ i ][ j ].x ) * ( pos.x - WaterSurface[ i ][ j ].x ) + ( pos.z - WaterSurface[ i ][ j ].z ) * ( pos.z - WaterSurface[ i ][ j ].z ) ;
			if( len <= 9 )
				WaterVelocity[ i ][ j ] = -0.15 ;
		}
}


void KeyPress( unsigned char key, int x, int y  )
{

}


int main( int argc, char** argv )
{
	glutInit( &argc, argv ) ;
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH ) ;
	glutInitWindowSize( 800, 600 ) ;
	glutCreateWindow( "Fast Water Simulation for Games Using Height Fields" ) ;
	
	InitDemo() ;

	glutDisplayFunc( Display ) ;
	glutReshapeFunc( Reshape ) ;
	glutMouseFunc( MouseButton ) ;
	glutPassiveMotionFunc( MousePassive ) ;
	glutKeyboardFunc( KeyPress ) ;
	glutTimerFunc( 16, Update, 0 ) ;

	glutMainLoop() ;
	return 0 ;
}