#ifndef __SKYDOME_H__
#define __SKYDOME_H__
#define GL_CLAMP_TO_EDGE 0x812F    //microsoft didn't define this!!


#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <GL/glpng.h>
#include <GL/glut.h>
using namespace glm ;


class SkyDome
{
private :
	vec3* ver ;    //vertices
	vec2* tex ;    //texture coordinate
	vec3 compass ;    //locate the vertex
	int grid_w, grid_h ;    //grid size

	GLuint texid ;
	GLushort* indices1 ;    //vertex indices1
	GLushort* indices2 ;    //vertex indices2

public :
	//half sphere=>phi_max set 90, sphere=>phi_max set 180
	bool Build( GLfloat radius, GLfloat phi_max, int slices, int stacks, const char* imgfile ) ;
	void Draw() ;
	
	SkyDome() ;
	~SkyDome() ;
} ;


#endif