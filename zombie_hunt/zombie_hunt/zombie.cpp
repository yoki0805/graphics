#include "zombie.h"



//ChineseZombie class
ChineseZombie::ChineseZombie()
{
	theta = 0, phi = 0, move = 0, hit = 0 ;
	respawn_flag = 0, respawn_timer = 0 ;
	x2_flag = 0, x2_timer = (float)(rand()%21) ; 
}


void ChineseZombie::Init( GLint name, GLuint tex_id, GLuint aim_id, GLuint x2_id )
{
	//init object
	this->name = name ;
	this->tex_id = tex_id ;
	this->aim_id = aim_id ;
	this->x2_id = x2_id ;
}


void ChineseZombie::SetZombie( vec3 pos, GLfloat move, GLfloat phi )
{
	this->pos = pos ;
	this->move = move ;
	this->phi = phi ;
}


int ChineseZombie::GetScore()
{
	return x2_flag ? 20 : 10 ;
}


void ChineseZombie::Draw()
{
	int rendermod ;
	glGetIntegerv( GL_RENDER_MODE, &rendermod ) ;

	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glAlphaFunc( GL_GREATER, 0.5 ) ;

	//scale 1.5
	glScalef( 1.5, 1.5, 1.5 ) ;
	glRotatef( theta, -1, 0, 0 ) ;    //dropping

	if( rendermod == GL_RENDER ) {
		//chinese zombie
		glPushMatrix() ;
		glBindTexture( GL_TEXTURE_2D, tex_id ) ;
		glBegin( GL_QUADS ) ;
		glTexCoord2f( 0, 0 ) ; glVertex2f( -1, 0 ) ;
		glTexCoord2f( chz_flip*1, 0 ) ; glVertex2f( 1, 0 ) ;
		glTexCoord2f( chz_flip*1, 1 ) ; glVertex2f( 1, 4 ) ;
		glTexCoord2f( 0, 1 ) ; glVertex2f( -1, 4 ) ;
		glEnd() ;
		glPopMatrix() ;

		//polygon offset
		glEnable( GL_POLYGON_OFFSET_FILL ) ;
		glPolygonOffset( -1, -1 ) ;

		if( x2_flag == 1 ) {
			//x2
			glPushMatrix() ;
			glTranslatef( -chz_flip*0.15, 3.8, 0 ) ;
			glBindTexture( GL_TEXTURE_2D, x2_id ) ;
			glBegin( GL_QUADS ) ;
			glTexCoord2f( 0, 0 ) ; glVertex2f( -0.5, -0.5 ) ;
			glTexCoord2f( 1, 0 ) ; glVertex2f( 0.5, -0.5 ) ;
			glTexCoord2f( 1, 1 ) ; glVertex2f( 0.5, 0.5 ) ;
			glTexCoord2f( 0, 1 ) ; glVertex2f( -0.5, 0.5 ) ;
			glEnd() ;
			glPopMatrix() ;
		}
	}

	glPushName( this->name ) ;
		//draw aim
		glPushMatrix() ;
		glTranslatef( chz_flip*0.175, 1.75, 0 ) ;
		glBindTexture( GL_TEXTURE_2D, aim_id ) ;
		glBegin( GL_QUADS ) ;
		glTexCoord2f( 0, 0 ) ; glVertex2f( -0.5, -0.5 ) ;
		glTexCoord2f( 1, 0 ) ; glVertex2f( 0.5, -0.5 ) ;
		glTexCoord2f( 1, 1 ) ; glVertex2f( 0.5, 0.5 ) ;
		glTexCoord2f( 0, 1 ) ; glVertex2f( -0.5, 0.5 ) ;
		glEnd() ;
		glPopMatrix() ;
	glPopName() ;

	glPopAttrib() ;
}


void ChineseZombie::Update( float dt )
{
	//zombie flip
	if( fabs( pos[0] ) > 18 ) {
		chz_flip *= -1 ;
	}
	pos[0] += chz_flip*move*dt ;    //moving

	//zombie jump
	if( phi > 360 ) {
		phi = 0 ;
	}

	if( hit == 0 ) {
		phi += 210*dt ;
		pos[1] = fabs( cos( phi*3.14159/180 ) ) ;    //jumpping
	}

	//zombie drop
	if( hit == 1 ) {
		theta += 180*dt ;    //dropping speed
		if( theta > 180 ) {    //stop dropping
			hit = 0 ;
			respawn_flag = 1 ;
			respawn_timer = (float)(rand()%4+2) ;    //2~5
			
			//x2
			x2_flag = 0 ;
			x2_timer = (float)(rand()%31) + respawn_timer ;
		}
	}

	//respawn
	if( respawn_flag == 1 ) {
		respawn_timer -= dt ;
		if( respawn_timer < 0 ) {
			respawn_flag = 0 ;
			theta = 0 ;
		}
	}

	//x2
	if( x2_flag == 0 ) {
		x2_timer -= dt ;
		if( x2_timer < 0 ) {
			x2_timer = 0 ;
			x2_flag = 1 ;
		}
	}
}



//Phantasma class
Phantasma::Phantasma()
{
	move = 0, hit = 0 ;
	respawn_flag = 0, respawn_timer = 0 ;
}


void Phantasma::Init( GLint name, GLuint tex_id, GLuint aim_id )
{
	//init object
	this->name = name ;
	this->tex_id = tex_id ;
	this->aim_id = aim_id ;
}


void Phantasma::SetPhantasma( vec3 pos, GLfloat move, GLfloat ground, GLfloat sky )
{
	this->pos = pos ;
	this->move = move ;
	this->ground_level = ground ;
	this->sky_level = sky ;
}


int Phantasma::GetScore()
{
	return 5 ;
}


void Phantasma::Draw()
{
	int rendermod ;
	glGetIntegerv( GL_RENDER_MODE, &rendermod ) ;

	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glEnable( GL_ALPHA_TEST ) ;
	glAlphaFunc( GL_GREATER, 0.5 ) ;

	//scale 1.5
	glScalef( 1.5, 1.5, 1.5 ) ;

	if( rendermod == GL_RENDER ) {
		//phantasma
		glPushMatrix() ;
		glBindTexture( GL_TEXTURE_2D, tex_id ) ;
		glBegin( GL_QUADS ) ;
		glTexCoord2f( 0, 0 ) ; glVertex2f( -1, 0 ) ;
		glTexCoord2f( phan_flip*1, 0 ) ; glVertex2f( 1, 0 ) ;
		glTexCoord2f( phan_flip*1, 1 ) ; glVertex2f( 1, 4 ) ;
		glTexCoord2f( 0, 1 ) ; glVertex2f( -1, 4 ) ;
		glEnd() ;
		glPopMatrix() ;
	}

	//polygon offset
	glEnable( GL_POLYGON_OFFSET_FILL ) ;
	glPolygonOffset( -1, -1 ) ;

	glPushName( this->name ) ;
		//draw aim
		glPushMatrix() ;
		glTranslatef( phan_flip*0.4, 1.5, 0 ) ;
		glBindTexture( GL_TEXTURE_2D, aim_id ) ;
		glBegin( GL_QUADS ) ;
		glTexCoord2f( 0, 0 ) ; glVertex2f( -0.5, -0.5 ) ;
		glTexCoord2f( 1, 0 ) ; glVertex2f( 0.5, -0.5 ) ;
		glTexCoord2f( 1, 1 ) ; glVertex2f( 0.5, 0.5 ) ;
		glTexCoord2f( 0, 1 ) ; glVertex2f( -0.5, 0.5 ) ;
		glEnd() ;
		glPopMatrix() ;
	glPopName() ;

	glPopAttrib() ;
}


void Phantasma::Update( float dt )
{
	//phantasma flip
	if( fabs( pos[0] ) > 15 ) {
		phan_flip *= -1 ;
	}
	pos[0] += phan_flip*move*dt ;    //moving

	//phantasma drop
	if( hit == 1 ) {
		pos[1] += 10*dt ;    //dropping speed
		if( pos[1] > sky_level+10 ) {    //stop dropping
			hit = 0 ;
			respawn_flag = 1 ;
			respawn_timer = (float)(rand()%4+2) ;    //2~5
		}
	}

	//respawn
	if( respawn_flag == 1 ) {
		respawn_timer -= dt ;
		if( respawn_timer < 0 ) {
			respawn_flag = 0 ;
			pos[1] = 0 ;
		}
	}
}