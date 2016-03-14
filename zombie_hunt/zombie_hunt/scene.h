#ifndef __SCENE_H__
#define __SCENE_H__


#include <ctime>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <gl/glpng.h>
#include <gl/glut.h>
using namespace glm ;


void InitScene() ;
void DrawScene() ;
void DrawTerrain() ;

void ProcessHit( GLint hits, GLuint* buffer ) ;
void UpdateScene( float dt ) ;


#endif