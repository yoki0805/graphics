#include <gl/glew.h>
#pragma comment( lib, "glew32.lib" )


bool InitFBO( GLuint* fbo, GLuint* texfbo, GLuint* dbo, int w, int h )
{
	//generate a framebuffer object
	glGenFramebuffersEXT( 1, fbo ) ;
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, *fbo ) ;

	//generate a texture for FBO
	glGenTextures( 1, texfbo ) ;
	glBindTexture( GL_TEXTURE_2D, *texfbo ) ;
	//set filter
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR ) ;
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR ) ;
	//set texture format
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 ) ;
	//framebuffer
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, *texfbo, 0 ) ;

	//generate zbuffer
	glGenRenderbuffersEXT( 1, dbo ) ;
	glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, *dbo ) ;
	glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, w, h ) ;
	glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, *dbo ) ;

	//back to default
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ) ;

	//check framebuffer object
	GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) ;
	if( status != GL_FRAMEBUFFER_COMPLETE_EXT ) {
		return false ;
	}
	return true ;
}