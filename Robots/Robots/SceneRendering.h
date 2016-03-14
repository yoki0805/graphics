#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gl/freeglut.h>


// lights
void SetLighting(GLfloat lp[]);

// shadow matrix
void SetShadowMatrix(glm::mat4& mat, const glm::vec4& lpos, const glm::vec4& plane);

// base
void SetBaseDisplayList();
void DrawBase(float halfSize);

void SetBulletDisplayList();
void DrawBullet(float halfSize);
void DrawBulletShadow(float halfSize);

// turret
void SetTurretDisplayList();
void DrawTurret(GLfloat halfSize, GLfloat thetaBase, GLfloat thetaBody, GLfloat thetaRader);
void DrawTurretShadow(GLfloat halfSize, GLfloat thetaBase, GLfloat thetaBody, GLfloat thetaRader);