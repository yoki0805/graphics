#include "scene.h"
#include "skydome.h"
#include "zombie.h"


//terrain
GLuint terrain_id ;
GLfloat ground_level = -5 ;
GLfloat sky_level = 2.5 ;

//skydome
SkyDome skydome ;
GLfloat sky_theta = 0 ;

//target
GLuint chz_id, phan_id ;
GLuint aim_id, x2_id ;

//zombie & phantasma
ChineseZombie chz[5] ;
Phantasma phan[5] ;
Target* target[10] ;

//total score
extern int score ;


void InitScene()
{
	//init skydome object
	skydome.Build( 50, 180, 30, 15, "data/skydome.png" ) ;

	//load texture
	pngSetStandardOrientation(1) ;
	terrain_id = pngBind( "data/terrain.png", PNG_BUILDMIPMAP, PNG_SOLID, NULL, GL_REPEAT, GL_LINEAR_MIPMAP_NEAREST, GL_LINEAR ) ;
	chz_id = pngBind( "data/chinese_zombie.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_REPEAT, GL_LINEAR, GL_LINEAR ) ;
	phan_id = pngBind( "data/phantasma.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_REPEAT, GL_LINEAR, GL_LINEAR ) ;
	aim_id = pngBind( "data/aim.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;
	x2_id = pngBind( "data/x2.png", PNG_NOMIPMAP, PNG_ALPHA, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;

	//init chinese zombie
	for( int i = 0 ; i < 5 ; ++i ) {
		chz[i].Init( i, chz_id, aim_id, x2_id ) ;
		chz[i].SetZombie( vec3( -10+5.0*i, 0, 0 ), 5, (20*i)%360 ) ;
		target[i] = &chz[i] ;
	}

	//init phantasma
	for( int i = 0 ; i < 5 ; ++i ) {
		phan[i].Init( i+5, phan_id, aim_id ) ;
		phan[i].SetPhantasma( vec3( -10+5.0*i, 0, 0 ), 3, ground_level, sky_level ) ;
		target[i+5] = &phan[i] ;
	}

	//random seed
	srand( (unsigned int)time(NULL) ) ;
}


void DrawScene()
{
	int rendermod ;
	glGetIntegerv( GL_RENDER_MODE, &rendermod ) ;

	if( rendermod == GL_RENDER ) {
		//draw terrain
		glPushMatrix() ;
		glTranslatef( 0, ground_level, -35 ) ;
		DrawTerrain() ;
		glPopMatrix() ;

		//draw sky dome
		glPushMatrix() ;
		glScalef( 1, 0.75, 1 ) ;
		glRotatef( sky_theta, 0, 1, 0 ) ;
		skydome.Draw() ;
		glPopMatrix() ;
	}

	//draw target
	for( int i = 0 ; i < 10 ; ++i )
	{
		glPushMatrix() ;
		if( i < 5 )
			glTranslatef( target[i]->pos[0], target[i]->pos[1]+ground_level, -20 ) ;
		else
			glTranslatef( target[i]->pos[0], target[i]->pos[1]+sky_level, -20 ) ;
		target[i]->Draw() ;
		glPopMatrix() ;
	}
/*
	for( int i = 0 ; i < 5 ; ++i ) {
		glPushMatrix() ;
		glTranslatef( chz[i].pos[0], chz[i].pos[1]+ground_level, -20 ) ;
		chz[i].Draw() ;
		glPopMatrix() ;
	}

	for( int i = 0 ; i < 3 ; ++i )	{
		glPushMatrix() ;
		glTranslatef( phan[i].pos[0], phan[i].pos[1]+sky_level, -20 ) ;
		phan[i].Draw() ;
		glPopMatrix() ;
	}
*/
}


void DrawTerrain()
{
	int min = -10, max = 10 ;
	float d = 12.0 / (max-min) ;

	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glBindTexture( GL_TEXTURE_2D, terrain_id ) ;
	glEnable( GL_TEXTURE_2D ) ;

	glPushMatrix() ;
	glScalef( 5, 5, 5 ) ;
	for( int i = min ; i < max ; ++i )
	{
		glBegin( GL_TRIANGLE_STRIP ) ;
		for( int j = min ; j <= max ; ++j )
		{
			glTexCoord2f( d*(i-min+1), d*(max-j) ) ;
			glVertex3i( i+1, 0, j ) ;
			glTexCoord2f( d*(i-min), d*(max-j) ) ;
			glVertex3i( i, 0, j ) ;
		}
		glEnd() ;
	}
	glPopMatrix() ;
	glPopAttrib() ;
}


void ProcessHit( GLint hits, GLuint* buffer )
{
	GLuint names ;
	GLuint* ptr = buffer ;

	for( int i = 0 ; i < hits ; ++i )
	{
		names = *ptr ;
		printf( "number of names for hit = %d\n", names ) ;
		++ptr ;

		printf( "z1 is %g, ", (float)*ptr/0xffffffff ) ;
		++ptr ;
		printf( "z2 is %g\n", (float)*ptr/0xffffffff ) ;
		++ptr ;

		printf( "the name is " ) ;
		for( int j = 0 ; j < names ; ++j ) {
			printf( "%d ", *ptr ) ;
			target[ (*ptr) ]->hit = 1 ;    //set flag
			score += target[ (*ptr) ]->GetScore() ;
			++ptr ;
		}
		printf( "\n" ) ;
	}
}


void UpdateScene( float dt )
{
	//sky
	if( sky_theta > 360 ) {
		sky_theta = 0 ;
	}

	//update target
	for( int i = 0 ; i < 10 ; ++i ) {
		target[i]->Update( dt ) ;
	}

	//update sky dome
	sky_theta += 1.5*dt ;
}