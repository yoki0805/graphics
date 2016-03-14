#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gl/glew.h>
#include <gl/glpng.h>
#include <gl/md2m.h>
#include <gl/gluit.h>
#include <gl/glut.h>
#pragma comment( lib, "glew32.lib" )
using namespace glm ;


//window size
int gw = 800 ;
int gh = 600 ;

//simple viewer
vec3 rot, eye ;    //viewer
ivec2 m_xy ;    //mouse position
ivec3 m_btn ;    //mouse button

//scene
GLuint texFloor ;
vec4 lightPosition = vec4( -10, 12, 10, 1 ) ;
vec3 teapotPosition[5] = { vec3( 3, 2.5, 3 ), vec3( -4, 2.5, 1 ), vec3( 5, 2.5, -2 ), vec3( -1, 2.5, -4 ), vec3( -1, 2.5, 6 ) } ;
int score = 0 ;

//md2 model
struct InfoMD2M {
	float scale ;
	vec3 center, size ;
	t3DModel* mdl ;
} ;

//my model
InfoMD2M yoshi ;
int yoshiPose = 0 ;
int yoshiOldPose = 0 ;
int yoshiLoopOver = 0 ;
float yoshiTheta = 0 ;
float yoshiSpeed = 0 ;
vec3 yoshiVector = vec3( 1, 0, 0 ) ;
vec3 yoshiPosition = vec3( 0, 0, 0 ) ;

//shadow map
GLuint fboLightView, depthLightView ;
GLuint shadowMapSize = 2048 ;
GLuint prog_shadowing ;


void InitMD2M( InfoMD2M* md2m, char* filename, char* pngname )
{
	#define max2(x,y) ((x)>(y)?(x):(y))
	#define max3(x,y,z) (max2(max2(x,y),max2(y,z)))

	//load MD2 model
	md2m->mdl = MD2MReadMD2( filename, pngname ) ;
	MD2MFindDimension( md2m->mdl, value_ptr( md2m->center ), value_ptr( md2m->size ) ) ;
	md2m->scale = max3( md2m->size.x, md2m->size.y, md2m->size.z ) ;

	MD2MSetLoop( md2m->mdl, GL_TRUE ) ;
	MD2MSetAnimation( md2m->mdl, yoshiPose ) ;
	MD2MSetAnimationSpeed( md2m->mdl, 10 ) ;
}


void InitShadowMapShader()
{
	extern GLuint setShaders( char*, char* ) ;
	prog_shadowing = setShaders( "shadowmap.vert", "shadowmap.frag" ) ;

	glUseProgram( prog_shadowing ) ;
	glUniform1i( glGetUniformLocation( prog_shadowing, "sceneMap" ), 0 ) ;  // scene use TU #0
	glUniform1i( glGetUniformLocation( prog_shadowing, "shadowMap" ), 3 ) ;    //set shadowmap to texture unit 3
	glUniform1f( glGetUniformLocation( prog_shadowing, "xPixelOffset" ), 1.0/gw ) ;    //set x pixel offset
	glUniform1f( glGetUniformLocation( prog_shadowing, "yPixelOffset" ), 1.0/gh ) ;    //set y pixel offset
	glUniform1i( glGetUniformLocation( prog_shadowing, "use_pcf"), 1 ) ;    //default use pcf
	glUniform1i( glGetUniformLocation( prog_shadowing, "textured_object" ), 1 ) ;    //default use textured object
	glUseProgram(0) ;    //disable shader for now
}


void GenerateShadowMap()
{
	//Try to use a texture depth component
	glGenTextures( 1, &depthLightView ) ;
	glBindTexture( GL_TEXTURE_2D, depthLightView ) ;
	
	//GL_LINEAR does not make sense for depth texture. However, next tutorial shows usage of GL_LINEAR and PCF. Using GL_NEAREST
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) ;
	
	//Remove artefact on the edges of the shadowmap
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP ) ;
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP ) ;
	
	//This is to allow usage of shadow2DProj function in the shader
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY ) ; 
	
	//No need to force GL_DEPTH_COMPONENT24, drivers usually give you the max precision if available 
	glTexImage2D( GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapSize, shadowMapSize, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, 0 ) ;
	
	//create a framebuffer object
	glGenFramebuffersEXT( 1, &fboLightView ) ;
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fboLightView ) ;
	
	//Instruct openGL that we won't bind a color texture with the currently binded FBO
	glDrawBuffer( GL_NONE ) ;
	glReadBuffer( GL_NONE ) ;
	
	//attach the texture to FBO depth attachment point
	glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,GL_TEXTURE_2D, depthLightView, 0 ) ;
	
	//check FBO status
	GLenum FBOstatus = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT ) ;
	if( FBOstatus != GL_FRAMEBUFFER_COMPLETE_EXT )
		printf( "GL_FRAMEBUFFER_COMPLETE_EXT failed, CANNOT use FBO\n" ) ;
	
	//switch back to window-system-provided framebuffer
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ) ;
}


void InitDemo()
{
	//opengl
	glClearColor( 0.5, 0.5, 0.5, 1.0 ) ;
	glClearDepth( 1.0 ) ;
	glEnable( GL_DEPTH_TEST ) ;
	glEnable( GL_CULL_FACE ) ;

	//load texture
	pngSetStandardOrientation(1) ;
	texFloor = pngBind( "data//floor.png", PNG_BUILDMIPMAP, PNG_SOLID, 0, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR_MIPMAP_LINEAR ) ;

	//load md2
	InitMD2M( &yoshi, "data//yoshi-tris.md2", "data//yoshi.png" ) ;

	//init shader
	InitShadowMapShader() ;
	GenerateShadowMap() ;    //generate shadow map

	//camera init
	rot = vec3( 30, 0, 0 ) ;
	eye = vec3( 0, 1, 35 ) ;

	//mouse init
	m_xy = ivec2( 0, 0 ) ;
	m_btn = ivec3( GLUT_UP, GLUT_UP, GLUT_UP ) ;
}


void ApplyInverse()
{
	//note that modelview has been restored to camera view AT THIS POINT!!
	mat4 m, mv ;
	glGetFloatv( GL_MODELVIEW_MATRIX, &m[0][0] ) ;

	mv = inverse( m ) ;
    glMultMatrixf( &mv[0][0] ) ;
}


void DrawLightPosition()
{
	glPushMatrix() ;
	glTranslatef( lightPosition.x, lightPosition.y, lightPosition.z ) ;
	glColor3f( 1, 1, 0 ) ;
	glutWireSphere( 0.2, 15, 7 ) ;
	glColor3f( 1, 1, 1 ) ;
	glPopMatrix() ;
}


void DrawFloor()
{
	glPushMatrix() ;
	glScalef( 1.5, 1.5, 1.5 ) ;
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glBindTexture( GL_TEXTURE_2D, texFloor ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex3i( -7.5, 0, 7.5 ) ;
	glTexCoord2f( 2, 0 ) ; glVertex3i( 7.5, 0, 7.5 ) ;
	glTexCoord2f( 2, 2 ) ; glVertex3i( 7.5, 0, -7.5 ) ;
	glTexCoord2f( 0, 2 ) ; glVertex3i( -7.5, 0, -7.5 ) ;
	glEnd() ;
	glPopAttrib() ;
	glPopMatrix() ;
}


void DrawWall()
{
	glPushMatrix() ;
	glPushAttrib( GL_ENABLE_BIT ) ;
	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE ) ;
	glEnable( GL_COLOR_MATERIAL ) ;
	glEnable( GL_LIGHTING ) ;
	glEnable( GL_LIGHT0 ) ;
	
	glColor3f( 1, 0, 0 ) ;
	glTranslatef( 4, 3, -6 ) ;
	glScalef( 3, 3, 0.25 ) ;
	glutSolidCube( 2.0 ) ;
	glPopAttrib() ;
	glPopMatrix() ;
}


void DrawYoshi()
{
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glPushMatrix() ;
	glTranslatef( yoshiPosition.x, 1, yoshiPosition.z ) ;
	glRotatef( yoshiTheta, 0, 1, 0 ) ;
	glScalef( 2.0/yoshi.scale, 2.0/yoshi.scale, 2.0/yoshi.scale ) ;
	glTranslatef( -yoshi.center.x, -yoshi.center.y, -yoshi.center.z ) ;
	glDisable( GL_CULL_FACE ) ;
	//glFrontFace( GL_CW ) ;
	yoshiLoopOver = MD2MInterpolate( yoshi.mdl ) ;
	//glFrontFace( GL_CCW ) ;
	glPopMatrix() ;
	glPopAttrib() ;
}


void DrawTeapot()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE ) ;
	glEnable( GL_COLOR_MATERIAL ) ;
	glEnable( GL_LIGHTING ) ;
	glEnable( GL_LIGHT0 ) ;
	glDisable( GL_CULL_FACE ) ;

	glColor3f( 1, 1, 0 ) ;
	for( int i = 0 ; i < 5 ; ++i ) {
		if( teapotPosition[i].y > -10 ) {
			glPushMatrix() ;
			glTranslatef( teapotPosition[i].x, teapotPosition[i].y, teapotPosition[i].z ) ;
			glutSolidTeapot( 0.3 ) ;
			glPopMatrix() ;
		}
	}
	glPopAttrib() ;
}


void DrawScore()
{
	//draw score
	for( int i = 0 ; i < 5 ; ++i )
	{
		if( teapotPosition[i].y <= -10 && teapotPosition[i].y >= -13 ) {
			glPushAttrib( GL_ENABLE_BIT ) ;
			glDisable( GL_LIGHTING ) ;
			setfont( "times roman", 24 ) ;
			glDisable( GL_TEXTURE_2D ) ;
			glColor3f( 1, 1, 0 ) ;
			Drawstr( teapotPosition[i].x, 3, teapotPosition[i].z, "10" ) ;
			glPopAttrib() ;
		}
	}

	//draw string
	BEGIN_2D_OVERLAY( 10,10 ) ;
	glColor3f( 1, 1, 0 ) ;
	drawstr( 1,1, "Score: %d", score ) ;
	END_2D_OVERLAY() ;
}


void DrawScene()
{
	DrawFloor() ;
	DrawYoshi() ;
}


void ShadowMapPass()
{
	//get the depth texture from light position
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fboLightView ) ;
	glClear( GL_DEPTH_BUFFER_BIT ) ;    //just clear the depth buffer, no output to colorbuffer

	//change the viewport size to same as shadow map size
	glViewport( 0, 0, shadowMapSize, shadowMapSize ) ;

	//save current projection
	glMatrixMode( GL_PROJECTION ) ; glPushMatrix() ; glLoadIdentity() ;
	gluPerspective( 95, 1.0f, 2.0f, 50.0f ) ;    //light projection

	//viewer: save current modelview (containing Viewing of camera)
	glMatrixMode( GL_MODELVIEW ) ; glPushMatrix() ; glLoadIdentity() ;
	gluLookAt( lightPosition.x, lightPosition.y, lightPosition.z, 0, 0, 0, 0, 1, 0 ) ;

	//disable color writes, and use flat shading for speed
	glColorMask( 0, 0, 0, 0 ) ;
	glShadeModel( GL_FLAT ) ;
	
	//disable texturingin the first pass (if any)
	//disable lighting too
	glPushAttrib( GL_ENABLE_BIT ) ;
	glDisable( GL_LIGHTING ) ;
	glDisable( GL_TEXTURE_2D ) ;

	//draw back faces into the shadow map
	//glCullFace( GL_FRONT ) ;
	//IMPORTANT: to remove moire pattern (from z-fighting)
	glPolygonOffset( 8.0, 4.0 ) ;
	glEnable( GL_POLYGON_OFFSET_FILL ) ;

	//draw scene
	DrawScene() ;
	DrawTeapot() ;
	DrawWall() ;
	glCullFace( GL_BACK ) ;
	glPopAttrib() ;

	//restore states
	glShadeModel( GL_SMOOTH ) ;
	glColorMask( 1, 1, 1, 1 ) ;

	glPopMatrix() ;    //restore Modelview to camera
	glMatrixMode( GL_PROJECTION ) ; glPopMatrix() ;  //restore current projection
}


void RenderingPass()
{
	//textureMatrix=biasMatrix*lightProjectionMatrix*lightViewMatrix*cameraViewInverse
	//store the coordinate conversion matrix to texture matrix[3]
	//to be used in shader
	glActiveTexture( GL_TEXTURE3 ) ;  // use the last texture unit

	glMatrixMode( GL_TEXTURE ) ; glLoadIdentity() ;
	static float b[16] = { 0.5, 0, 0, 0, 0, 0.5, 0, 0, 0, 0, 0.5, 0, 0.5, 0.5, 0.5, 1 } ;
	glMultMatrixf(b) ;  //scale and bias
	gluPerspective( 95, 1.0f, 2.0f, 50.0f ) ;    //light projection
	gluLookAt( lightPosition.x, lightPosition.y, lightPosition.z, 0, 0, 0, 0, 1, 0 ) ;
	ApplyInverse() ;  //camera view inverse

	///////////////////////////////////////////////////////////////////////////////////////
	//back to glut buffers
	glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 ) ;
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
	glViewport( 0, 0, gw, gh ) ;

	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity() ;

	//viewer
	glTranslatef( -eye.x, -eye.y, -eye.z ) ;
	glRotatef( rot.x, 1, 0, 0 ) ;
	glRotatef( rot.y, 0, 1, 0 ) ;
	glRotatef( rot.z, 0, 0, 1 ) ;
	
	//bind & enable shadow map texture
	glActiveTexture( GL_TEXTURE3 ) ;    //for shadow map
	glBindTexture( GL_TEXTURE_2D, depthLightView ) ;
	
	glUseProgram( prog_shadowing ) ;
	glActiveTexture( GL_TEXTURE0 ) ;    //use textured object
	DrawScene() ;    //draw scene
	glUseProgram(0) ;

	//draw light position without shader
	DrawLightPosition() ;
}


void Display()
{
	ShadowMapPass() ;    //pass1
	RenderingPass() ;    //pass2

	DrawTeapot() ;    //draw in fixed pipeline
	DrawWall() ;
	DrawScore() ;

	glutSwapBuffers() ;
}


void Reshape( int w, int h )
{
	glViewport( 0, 0, w, h ) ;
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	double aspect = (double)w / (double)h ;
	gluPerspective( 45, aspect, 0.1, 50 ) ;
	gw = w, gh = h ;

	//change the uniforms
	glUseProgram( prog_shadowing ) ;
	glUniform1f( glGetUniformLocation( prog_shadowing, "xPixelOffset" ), 1.0/gw ) ;    //set x pixel offset
	glUniform1f( glGetUniformLocation( prog_shadowing, "yPixelOffset" ), 1.0/gh ) ;    //set y pixel offset
	glUseProgram(0) ;    //disable shader for now
}


void KeyPress( unsigned char key, int x, int y )
{
	if( key == 27 ) {
		exit(0) ;
	}

	if( key == ' ' ) {
		if( yoshiPose != 6 ) {
			yoshiOldPose = yoshiPose ;
			yoshiPose = 6 ;
			MD2MSetLoop( yoshi.mdl, GL_FALSE ) ;
			MD2MSetAnimation( yoshi.mdl, yoshiPose ) ;
			MD2MSetAnimationSpeed( yoshi.mdl, 8 ) ;
		}
	}
}


void  KeySpecialPress( int key, int x, int y )
{
	if( key == GLUT_KEY_UP ) {
		yoshiSpeed += 0.015 ;
		if( yoshiSpeed > 0.075 )    yoshiSpeed = 0.075 ;
		MD2MSetAnimationSpeed( yoshi.mdl, yoshiSpeed*150 ) ;
	}
	
	if( key == GLUT_KEY_DOWN ) {
		yoshiSpeed -= 0.01 ;
		if( yoshiSpeed < 0 )    yoshiSpeed = 0 ;
	}

	if( key == GLUT_KEY_RIGHT ) {
		yoshiTheta -= 3.0 ;
	}
	
	if( key == GLUT_KEY_LEFT )	 {
		yoshiTheta += 3.0 ;
	}

	//check animation
	if( yoshiSpeed > 0 ) {
		if( yoshiPose == 0 ) {
			yoshiPose = 1 ;
			MD2MSetAnimation( yoshi.mdl, yoshiPose ) ;
		}
	} else {
		if( yoshiPose == 1 ) {
			yoshiPose = 0 ;
			MD2MSetAnimationSpeed( yoshi.mdl, 10 ) ;
			MD2MSetAnimation( yoshi.mdl, yoshiPose ) ;
		}
	}
}


void MouseButton( int button, int state, int x, int y )
{
	//update mouse state
	m_btn[ button ] = state ;
	m_xy = vec2( x, y ) ;
}


void MouseMotion( int x, int y )
{
	if( m_btn[ GLUT_LEFT_BUTTON ] == GLUT_DOWN )
	{
		//rotae
		rot.x += ( y - m_xy[1] ) * 0.5 ;
		rot.y += ( x - m_xy[0] ) * 0.5 ;
		//angle lock
		if( rot.x > 90 )  rot.x = 90 ;
		if( rot.x < -90 )  rot.x = -90 ;
	} else if( m_btn[ GLUT_RIGHT_BUTTON ] == GLUT_DOWN )	{
		//translate
		eye.z -= ( y - m_xy[1] ) * 0.05 ;
		if( eye.z < 1 )    eye.z = 1 ;
	}

	m_xy = vec2( x, y ) ;
	glutPostRedisplay() ;
}


void Grab()
{
	for( int i = 0 ; i < 5 ; ++i ) {
		if( yoshiPose == 6 && length( teapotPosition[i] - yoshiPosition ) < 2.7 ){
			teapotPosition[i].y = -10 ;    //hiding
			score += 10 ;
		}

		//count down
		if( teapotPosition[i].y <= -10 && teapotPosition[i].y >= -13 ) {
			teapotPosition[i].y -= 0.016 ;
		}
	}
}


void MyTimer( int dummy )
{
	//get position
	yoshiVector = glm::rotateY( vec3( 1, 0, 0 ), yoshiTheta ) ;
	yoshiPosition += yoshiSpeed * yoshiVector ;

	//deal with jumping
	if( yoshiPose == 6 ) {
		if( yoshiLoopOver ) {
			MD2MSetLoop( yoshi.mdl, GL_TRUE ) ;
			MD2MSetAnimation( yoshi.mdl, yoshiOldPose ) ;
			MD2MSetAnimationSpeed( yoshi.mdl, yoshiSpeed*150 ) ;
			yoshiPose = yoshiOldPose ;
		}
	}

	Grab() ;    //eat the teapot

	glutTimerFunc( 16, MyTimer, 0 ) ;
	glutPostRedisplay() ;
}


int main( int argc, char** argv )
{
	glutInit( &argc, argv ) ;
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH ) ;
	glutInitWindowSize( gw, gh ) ;
	glutCreateWindow( "GLSL Shadowing Demo [Key: Space Bar | Arrow Keys]" ) ;
  
	glewInit() ;
	InitDemo() ;

	glutDisplayFunc( Display ) ;
	glutReshapeFunc( Reshape ) ;
	glutKeyboardFunc( KeyPress ) ;
	glutSpecialFunc( KeySpecialPress ) ;
	glutMouseFunc( MouseButton ) ;
	glutMotionFunc( MouseMotion ) ;
	glutTimerFunc( 16, MyTimer, 0 ) ;

	glutMainLoop() ;
	return 0 ;
}