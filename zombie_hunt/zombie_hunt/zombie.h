#ifndef __ZOMBIE_H__
#define __ZOMBIE_H__


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/glpng.h>
#include <gl/glut.h>
using namespace glm ;


class Target
{
public :
	GLint name ;
	GLint hit ;
	vec3 pos ;
	
	virtual void Draw() = 0 ;
	virtual void Update( float dt ) = 0 ;    //behavior
	virtual int GetScore() = 0 ;
} ;


class ChineseZombie : public Target
{
private :
	GLuint tex_id, aim_id, x2_id ;
	GLfloat theta, phi, move ;
	GLfloat respawn_timer, x2_timer ;
	GLint respawn_flag, x2_flag ;
	
public :
	void Draw() ;
	void Update( float dt ) ;
	inline int GetScore() ;

	void Init( GLint name, GLuint tex_id, GLuint aim_id, GLuint x2_id ) ;
	void SetZombie( vec3 pos, GLfloat move, GLfloat phi ) ;
	ChineseZombie() ;
} ;


class Phantasma : public Target
{
private :
	GLuint tex_id, aim_id ;
	GLfloat move, ground_level, sky_level ;
	GLfloat respawn_timer ;
	GLint respawn_flag ;

public :
	void Draw() ;
	void Update( float dt ) ;
	inline int GetScore() ;

	void Init( GLint name, GLuint tex_id, GLuint aim_id ) ;
	void SetPhantasma( vec3 pos, GLfloat move, GLfloat ground, GLfloat sky ) ;
	Phantasma() ;
} ;


//flip flag
static GLint chz_flip = 1 ;
static GLint phan_flip = -1 ;


#endif