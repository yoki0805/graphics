#include "skydome.h"


SkyDome::SkyDome()
{
	ver = NULL ;
	tex = NULL ;
	indices1 = NULL ;
	indices2 = NULL ;
	texid = 0 ;
}


SkyDome::~SkyDome()
{
	if( ver != NULL )    delete[] ver ;
	if( tex != NULL )    delete[] tex ;
	if( indices1 != NULL )    delete[] indices1 ;
	if( indices2 != NULL )    delete[] indices2 ;
	if( texid != 0 )    glDeleteTextures( 1, &texid ) ;
}


bool SkyDome::Build( GLfloat radius, GLfloat phi_max, int slices, int stacks, const char* imgfile )
{
	//get grid_w & grid_h
	grid_w = slices+1, grid_h = stacks ;

	//allocate memory
	if( ver != NULL )    delete[] ver ;
	ver = new vec3[ grid_w*grid_h+1 ] ;    //the last vertex is north pole

	if( tex != NULL )    delete[] tex ;
	tex = new vec2[ grid_w*grid_h+1 ] ;    //texture coordinate

	if( indices1 != NULL )    delete[] indices1 ;
	indices1 = new GLushort[ grid_w*grid_h*2 ] ;    //indices1

	if( indices2 != NULL )    delete[] indices2 ;
	indices2 = new GLushort[ grid_w+1 ] ;    //indices2

	//load texture
	if( texid != 0 )    glDeleteTextures( 1, &texid ) ;
	pngSetStandardOrientation(1) ;
	texid = pngBind( imgfile, PNG_NOMIPMAP, PNG_SOLID, 0,  GL_CLAMP_TO_EDGE, GL_LINEAR, GL_LINEAR ) ;

	//get spacing
	float delta_theta = 360.0f / slices ;    //for angle
	float delta_phi = phi_max / stacks ;
	float delta_s = 1.0f / slices ;    //for texture coordinate
	float delta_t = 1.0f / (stacks-1) ;

	//angles ;
	float theta = 0 ;    //yaw
	float phi = 90-phi_max ;    //pitch
	float tex_s = 0 ;    //texcoord s 
	float tex_t = 0 ;    //texcoord t

	//calculate
	for( int i = 0 ; i < grid_h ; ++i )
	{
		compass = rotateZ( vec3( radius, 0, 0 ), phi ) ;    //get compass
		for( int j = 0 ; j < grid_w ; ++j )
		{
			ver[ i*grid_w+j ] = rotateY( compass, theta ) ;    //get vertex
			tex[ i*grid_w+j ] = vec2( tex_s, tex_t ) ;    //get texture coordinate

			theta += delta_theta ;    //add angle
			tex_s += delta_s ;    //add spacing
		}
		phi += delta_phi ;    //add angle
		tex_t += delta_t ;    //add spacing
		theta = 0 ;    //just clear
		tex_s = 0 ;    //just clear
	}
	ver[ grid_w*grid_h ] = vec3( 0, radius, 0 ) ;    //set north pole
	tex[ grid_w*grid_h ] = vec2( 0.5, 1.0 ) ;    //set north pole texture coordinate

	//calculate indices order
	for( int i = 0 ; i < grid_h-1 ; ++i )    //indices1
	{
		for( int j = 0 ; j < grid_w ; ++j )
		{
			int n = i*grid_w+j ;
			indices1[ 2*n ] = n ;
			indices1[ 2*n+1 ] = n+grid_w ;
		}
	}

	for( int i = 0 ; i < grid_w+1 ; ++i ) {    //indices2
		indices2[i] = grid_w*grid_h-i ;
	}

	return true ;
}


void SkyDome::Draw()
{
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glDisable( GL_LIGHTING ) ;

	//set vertex array
	glEnableClientState( GL_VERTEX_ARRAY ) ;
	glVertexPointer( 3, GL_FLOAT, 0, value_ptr( ver[0] ) ) ;

	//set texture coord array
	glEnableClientState( GL_TEXTURE_COORD_ARRAY ) ;
	glTexCoordPointer( 2, GL_FLOAT, 0, value_ptr( tex[0] ) ) ;

	//draw
	glBindTexture( GL_TEXTURE_2D, texid ) ;
	for( int i = 0 ; i < grid_h-1 ; ++i ) {    //indices1
		glDrawElements( GL_TRIANGLE_STRIP, 2*grid_w, GL_UNSIGNED_SHORT, &indices1[ 2*grid_w*i ] ) ;
	}
	for( int i = 0 ; i < grid_w+1 ; ++i ) {    //indices2
		glDrawElements( GL_TRIANGLE_FAN, grid_w+1, GL_UNSIGNED_SHORT, &indices2[0] ) ;
	}

	glDisableClientState( GL_VERTEX_ARRAY ) ;
	glDisableClientState( GL_NORMAL_ARRAY ) ;
	glDisableClientState( GL_TEXTURE_COORD_ARRAY ) ;
	glPopAttrib() ;
}