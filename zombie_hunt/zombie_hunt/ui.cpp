#include "ui.h"

//2d overlay
GLfloat overlay_size[2] ;
static int width, height ;    //only in 2d overlay

//sight
GLuint sight_id ;
GLfloat sight_pos[2] ;

//grass
GLuint grass_id ;
GLfloat grass_sway = 0 ;
GLint grass_turn = 1 ;

//clock
GLuint clock_id, sec_id ;
GLfloat sec_theta = 0 ;

//score board
GLuint board_id ;

//end
GLuint end_id ;
GLuint zombie_kiss ;


void InitUI()
{
	//load texture
	pngSetStandardOrientation(1) ;
	sight_id = pngBind( "data/sight.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
	grass_id = pngBind( "data/grass.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
	clock_id = pngBind( "data/clock.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
	sec_id = pngBind( "data/sec_pointer.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
	board_id = pngBind( "data/scroll.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
	end_id = pngBind( "data/endscreen.png", PNG_NOMIPMAP, PNG_SOLID, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
	zombie_kiss = pngBind( "data/zombie_kiss.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
}


void Reshape2dOverlay( int w, int h )
{
	//init overlay size
	overlay_size[0] = 10 , overlay_size[1] = 10 ;

	 //reshape 2d overlay
	if( w > h )
		overlay_size[0] *= (float)w/h ;
	else 
		overlay_size[1] *= (float)h/w ;

	//get window size
	width = w, height = h ;
}


void DrawSight( int x, int y )
{
	//get sight pos
	sight_pos[0] = ((float)x/width) * overlay_size[0] ;
	sight_pos[1] = (1.0f - ((float)y/height)) * overlay_size[1] ;

	//draw sight
	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glAlphaFunc( GL_GREATER, 0.5 ) ;
		
	GL_BEGIN_2D_OVERLAY( overlay_size[0], overlay_size[1] ) ;
	glTranslatef( sight_pos[0], sight_pos[1], 0 ) ;
	glBindTexture( GL_TEXTURE_2D, sight_id ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( -0.5, -0.5 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 0.5, -0.5 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 0.5, 0.5 ) ;
	glTexCoord2f( 0, 1 ) ; glVertex2f( -0.5, 0.5 ) ;
	glEnd() ;
	GL_END_2D_OVERLAY() ;

	glPopAttrib() ;
}


void DrawGameUI()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glAlphaFunc( GL_GREATER, 0.75 ) ;
	GL_BEGIN_2D_OVERLAY( overlay_size[0], overlay_size[1] ) ;

	//draw grass
	DrawGrass() ;
	DrawClock() ;
	DrawScoreBoard() ;

	GL_END_2D_OVERLAY() ;
	glPopAttrib() ;
}


void DrawGrass()
{
	glPushMatrix() ;
	glScalef( overlay_size[0], overlay_size[0]/4, 0 ) ;
	glBindTexture( GL_TEXTURE_2D, grass_id ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( 0, 0 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 1, 0 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 1-grass_sway, 1 ) ;    //swaing
	glTexCoord2f( 0, 1 ) ; glVertex2f( 0-grass_sway, 1 ) ;
	glEnd() ;
	glPopMatrix() ;
}


void DrawClock()
{
	glPushMatrix() ;
	//draw clock base
	glTranslatef( overlay_size[0]-1, overlay_size[1]-1, 0 ) ;
	glBindTexture( GL_TEXTURE_2D, clock_id ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( -1, -1 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 1, -1 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 1, 1 ) ;
	glTexCoord2f( 0, 1 ) ; glVertex2f( -1, 1 ) ;
	glEnd() ;

	//draw second pointer
	glRotatef( sec_theta, 0, 0, -1 ) ;
	glPushMatrix() ;
	glTranslatef( 0, 0.39, 0 ) ;
	glScalef( 0.125, 0.9, 0 ) ;
	glBindTexture( GL_TEXTURE_2D, sec_id ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( -0.5, -0.5 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 0.5, -0.5 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 0.5, 0.5 ) ;
	glTexCoord2f( 0, 1 ) ; glVertex2f( -0.5, 0.5 ) ;
	glEnd() ;
	glPopMatrix() ;

	glPopMatrix() ;
}


void DrawScoreBoard()
{
	glPushMatrix() ;
	//draw score board
	glTranslatef( overlay_size[0]-1, overlay_size[1]-3, 0 ) ;
	glBindTexture( GL_TEXTURE_2D, board_id ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( -1, -1 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 1, -1 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 1, 1 ) ;
	glTexCoord2f( 0, 1 ) ; glVertex2f( -1, 1 ) ;
	glEnd() ;
	glPopMatrix() ;
}


void UpdateGameUI( float dt )
{
	//sway grass
	if( fabs( grass_sway ) > 0.015 ) {
		grass_turn *= -1 ;
	}

	//clock timing
	if( sec_theta > 360 ) {
		sec_theta = 0 ;
	}

	grass_sway += grass_turn*0.025*dt ;    //grass waving
	sec_theta += 6*dt ;
}


void DrawEndScreen( int score )
{
	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glAlphaFunc( GL_GREATER, 0.75 ) ;
	GL_BEGIN_2D_OVERLAY( overlay_size[0], overlay_size[1] ) ;

	//background
	glBindTexture( GL_TEXTURE_2D, end_id ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( 0, 0 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( overlay_size[0], 0 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( overlay_size[0], overlay_size[1] ) ;
	glTexCoord2f( 0, 1 ) ; glVertex2f( 0, overlay_size[1] ) ;
	glEnd() ;

	glPushMatrix() ;
	glScalef( 0.5, 0.77, 1 ) ;
	glBindTexture( GL_TEXTURE_2D, zombie_kiss ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( 0, 0 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( overlay_size[0], 0 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( overlay_size[0], overlay_size[1] ) ;
	glTexCoord2f( 0, 1 ) ; glVertex2f( 0, overlay_size[1] ) ;
	glEnd() ;
	glPopMatrix() ;
	
	GL_END_2D_OVERLAY() ;
	glPopAttrib() ;
}