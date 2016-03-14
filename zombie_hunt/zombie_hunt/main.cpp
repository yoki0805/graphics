#include <windows.h>
#include <mmsystem.h>
#pragma comment( lib,"winmm.lib" )

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/glpng.h>
#include <gl/glut.h>
#include "scene.h"
#include "ui.h"
using namespace glm ;


//global vars
int width = 800, height = 600 ;    //window size
int time_interval = 16 ;    //update interval

//mouse
ivec3 m_btn = ivec3(1) ;    //mouse button
ivec2 m_pos = ivec2(0) ;    //mouse position

//game rule
int score = 0 ;
float timeup = 60 ;
int game_mode = 1 ;


void InitDemo()
{
	glClearColor( 0.4, 0.4, 0.4, 1.0 ) ;
	glClearDepth( 1.0 ) ;
	glEnable( GL_DEPTH_TEST ) ;
	glEnable( GL_CULL_FACE ) ;

	//init ui
	InitUI() ;
	InitScene() ;

	//hide cursor
	glutSetCursor( GLUT_CURSOR_NONE ) ; 

	//play music
	int i = 0 ;
    MCI_OPEN_PARMS mciOpen ;
    mciOpen.lpstrDeviceType = L"mpegvideo" ;
    mciOpen.lpstrElementName = L"data/MitiS_Endeavours.mp3" ;    
    mciSendCommand( 0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen ) ;

    MCI_STATUS_PARMS mciStatusParms ;
    mciStatusParms.dwItem = MCI_STATUS_LENGTH ;
    mciSendCommand( mciOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms ) ;   
    int curLength = mciStatusParms.dwReturn ;

    MCI_PLAY_PARMS mciPlay ;
    mciSendCommand( mciOpen.wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD)&mciPlay ) ;
}


void Picking( int x, int y )
{
	//start picking
	GLint viewport[4] ;
	GLfloat proj[16] ;

	glGetIntegerv( GL_VIEWPORT, viewport ) ;
	glGetFloatv( GL_PROJECTION_MATRIX, proj ) ;

	glMatrixMode( GL_PROJECTION ) ;
	glPushMatrix() ;
	glLoadIdentity() ;

	//create 5x5 pixel picking region near cursor location
	gluPickMatrix( (double)x, (double)(viewport[3]-y), 5, 5, viewport ) ;
	glMultMatrixf( proj ) ;

	glMatrixMode( GL_MODELVIEW ) ;

	GLuint select_buf[32] ;
	glSelectBuffer( 32, select_buf) ;
	glRenderMode( GL_SELECT ) ;
	glInitNames() ; 
	DrawScene() ;

	//restore PROJECTION matrix
	glMatrixMode( GL_PROJECTION ) ;
	glPopMatrix() ;

	GLint hits ;
	hits = glRenderMode( GL_RENDER ) ;

	//process hit
	if( hits == 0 )  return ;    //no hit
	ProcessHit( hits, select_buf ) ;	
}


void Display()
{
	int rendermod ;
	glGetIntegerv( GL_RENDER_MODE, &rendermod ) ;

	//render
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity() ;
	
	if( game_mode == 1 )
	{
		DrawScene() ;    //scene
		if( rendermod == GL_RENDER ) {
			DrawGameUI() ;
			DrawSight( m_pos.x, m_pos.y ) ;
		} 
	}
	else if( game_mode == 2 )
	{
		DrawEndScreen( score ) ;
		DrawSight( m_pos.x, m_pos.y ) ;
	}

	glutSwapBuffers() ;
}


void Reshape( int w, int h )
{
	glViewport( 0, 0, w, h ) ;
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	gluPerspective( 45, (double)w/h, 0.1, 150 ) ;

	Reshape2dOverlay( w, h ) ;  //reshap 2d overlay
	width = w, height = h ;
}


void GameLogic()
{
	static float dt = (float)time_interval * 0.001 ;    //ms to sec

	//if game started..
	if( game_mode == 1 ) {
		UpdateScene( dt ) ;
		UpdateGameUI( dt ) ;
		timeup -= dt ;
	}

	//end screen
	if( timeup < 0 && game_mode != 2 ) {
		printf( "Your Score: %d\n", score ) ;
		game_mode = 2 ;
	}
}


void Update( int dump )
{
	glutTimerFunc( time_interval, Update, 0 ) ;
	GameLogic() ;
	glutPostRedisplay() ;
}


void MouseButton( int button, int status, int x, int y )
{
	//update mouse state
	m_btn[ button ] = status ;
	m_pos[0] = x, m_pos[1] = y ;

	//picking
	if( m_btn[ GLUT_LEFT_BUTTON ] == GLUT_DOWN ) {
		Picking( m_pos.x, m_pos.y ) ;
	}
}


void MouseMotion( int x, int y )
{
	//get mouse position
	m_pos[0] = x, m_pos[1] = y ;
}


void MousePassive( int x, int y )
{
	//get mouse position
	m_pos[0] = x, m_pos[1] = y ;
}


int main( int argc, char** argv )
{
	glutInit( &argc, argv ) ;
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH ) ;
	glutInitWindowSize( width, height ) ;
	glutCreateWindow( "Zombie Hunt Demo" ) ;
	
	InitDemo() ;

	glutDisplayFunc( Display ) ;
	glutReshapeFunc( Reshape ) ;
	glutMouseFunc( MouseButton ) ;
	glutMotionFunc( MouseMotion ) ;
	glutPassiveMotionFunc( MousePassive ) ;
	glutTimerFunc( time_interval, Update, 0 ) ;
	
	glutMainLoop() ;
	return 0 ;
}