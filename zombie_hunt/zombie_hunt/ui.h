#ifndef __UI_H__
#define __UI_H__


#include <cmath>
#include <gl/glpng.h>
#include <gl/glut.h>


#define GL_BEGIN_2D_OVERLAY(w,h) \
{ \
	glMatrixMode( GL_PROJECTION ) ; \
	glPushMatrix() ; \
	glLoadIdentity() ; \
	gluOrtho2D( 0, w, 0, h ) ; \
	glMatrixMode( GL_MODELVIEW ) ; \
	glPushMatrix() ; \
	glLoadIdentity() ; \
	glPushAttrib( GL_ENABLE_BIT ) ; \
	glDisable( GL_DEPTH_TEST ) ; \
}


#define GL_END_2D_OVERLAY() \
{ \
	glPopAttrib() ; \
	glMatrixMode( GL_PROJECTION ) ; \
	glPopMatrix() ; \
	glMatrixMode( GL_MODELVIEW ) ; \
	glPopMatrix() ; \
}


//functions
void InitUI() ;
void Reshape2dOverlay( int w, int h ) ;
void DrawSight( int x, int y ) ;

void DrawGameUI() ;
void DrawGrass() ;
void DrawClock() ;
void DrawScoreBoard() ;

void UpdateGameUI( float dt ) ;
void DrawEndScreen( int score ) ;


#endif