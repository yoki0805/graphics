#include <windows.h>
#include <mmsystem.h>
#include <ctime>
#include <deque>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <gl/glpng.h>
#include <gl/glut.h>
#pragma comment( lib,"winmm.lib" )
using namespace glm ;
using namespace std ;


typedef struct tagParticle
{
	vec3 pos, vel, color ;
	int birth_time ;
	deque<vec3> trail ;
	bool exploded ;
	float intensity ;
} Particle ;


vec3 color_table[12] = {
	vec3( 1, 0.5, 0.5 ), vec3( 1, 0.75, 0.5 ), vec3( 1, 1, 0.5 ), vec3( 0.75, 1, 0.5 ), 
	vec3( 0.5, 1, 0.5 ), vec3( 0.5, 1, 0.75 ), vec3( 0.5, 1, 1 ), vec3( 0.5, 0.75, 1 ), 
	vec3( 0.5, 0.5, 1 ), vec3( 0.75, 0.5, 1 ), vec3( 1, 0.5, 1 ), vec3( 1, 0.5, 0.75 )
} ;


int width = 800, height = 600 ;    //window size
int time_interval = 16 ;    //update time  interval

vec3 gravity( 0, -1, 0 ) ;    //gravity
deque<Particle> particles ;    //all particles
int current_time ;

const float max_life = 7.5 ;    //particle life time
const int max_trail = 15 ;    //trail size
const int max_spark = 15 ;    //spark size

GLuint texid ;    //texture


double frand( double a, double b )
{
	double r = b-a ;
	return a+(double)rand()/RAND_MAX*r ;
}


Particle NewParticle()
{
	float start_x, start_y ;
	Particle p ;

	p.birth_time = glutGet( GLUT_ELAPSED_TIME ) ;
	start_x = frand( -10, 10 ) ;
	p.pos = vec3( start_x, 0, 0 ) ;

	start_x = frand( -0.3, 0.3 ) ;
	start_y = frand( -0.5, 0.5 ) ;
	p.vel = vec3( start_x, 5.5+start_y, 0 ) ;

	p.color = color_table[ (p.birth_time/500)%12 ] ;
	p.intensity = 1.0 ;
	p.trail.push_back( p.pos ) ;
	p.exploded = false ;
	return p ;
}


void Explode( Particle& p )
{
	static bool firstcall = true ;
	static vec3 velocity[max_spark] ;
	Particle spark[max_spark] ;

	//init velocities
	if( firstcall )	{
		for( int i = 0 ; i < max_spark ; ++i ) {
			float theta = (360.0/max_spark)*i ;
			velocity[i] = vec3( rotate( vec4( 2.5, 0, 0, 0 ), theta, vec3( 0, 0, 1 ) ) ) ;
		}
		firstcall = false ;
	}

	//set sparks
	for( int i = 0 ; i < max_spark ; ++i ) {
		spark[i].vel = velocity[i] + p.vel ;
		spark[i].pos = p.pos ;
		spark[i].birth_time = p.birth_time ;
		spark[i].color = p.color ;
		spark[i].intensity = p.intensity ;
		spark[i].trail.push_back( p.pos ) ;
		spark[i].exploded = true ;
	}

	//add to particles
	for( int i = 0 ; i < max_spark ; ++i ) {
		particles.push_back( spark[i] ) ;
	}
}


float LifeTime( const Particle& p )
{
	return (current_time - p.birth_time) * 0.001 ;
}


void InitDemo()
{
	//opengl
	glClearColor( 0, 0, 0, 1 ) ;
	glClearDepth( 1.0 ) ;
	glEnable( GL_DEPTH_TEST ) ;
	glEnable( GL_CULL_FACE ) ;
	glEnable( GL_LIGHT0 ) ;

	//random seed
	srand( (unsigned int)time(NULL) ) ;

	//texture
	pngSetStandardOrientation(1) ;
	texid = pngBind( "Tamsui.png", PNG_NOMIPMAP, PNG_SOLID, NULL, GL_CLAMP, GL_LINEAR, GL_LINEAR ) ;

	//play music
	int i = 0 ;
    MCI_OPEN_PARMS mciOpen ;
    mciOpen.lpstrDeviceType = L"mpegvideo" ;
    mciOpen.lpstrElementName = L"HearMeCry.mp3" ;    
    mciSendCommand( 0, MCI_OPEN, MCI_OPEN_TYPE | MCI_OPEN_ELEMENT, (DWORD)&mciOpen ) ;

    MCI_STATUS_PARMS mciStatusParms ;
    mciStatusParms.dwItem = MCI_STATUS_LENGTH ;
    mciSendCommand( mciOpen.wDeviceID, MCI_STATUS, MCI_STATUS_ITEM, (DWORD)&mciStatusParms ) ;   
    int curLength = mciStatusParms.dwReturn ;

    MCI_PLAY_PARMS mciPlay ;
    mciSendCommand( mciOpen.wDeviceID, MCI_PLAY, MCI_NOTIFY, (DWORD)&mciPlay ) ;
}


void DrawBackground()
{
	glPushAttrib( GL_ENABLE_BIT ) ;
	glEnable( GL_TEXTURE_2D ) ;
	glBindTexture( GL_TEXTURE_2D, texid ) ;

	glPushMatrix() ;
	glTranslatef( 0, 0, -10 ) ;
	glScalef( 7.5, 4.5, 0 ) ;
	glBegin( GL_QUADS ) ;
	glTexCoord2f( 0, 0 ) ; glVertex2f( -1, -1 ) ;
	glTexCoord2f( 1, 0 ) ; glVertex2f( 1, -1 ) ;
	glTexCoord2f( 1, 1 ) ; glVertex2f( 1, 1 ) ;
	glTexCoord2f( 0, 1 ) ; glVertex2f( -1, 1 ) ;
	glEnd() ;
	glPopMatrix() ;
	glPopAttrib() ;
}


void DrawParticles()
{
	glPushAttrib( GL_ALL_ATTRIB_BITS ) ;
	glDisable( GL_DEPTH_TEST ) ;

	glEnable( GL_POINT_SMOOTH ) ;
    glPointSize(10) ;
	glLineWidth(5) ;

	glBlendFunc( GL_SRC_ALPHA, GL_ONE ) ;
	glEnable( GL_BLEND ) ;

	for( size_t i = 0 ; i < particles.size() ; ++i )
	{
		Particle& p = particles[i] ;
		float life = LifeTime( p ) ;
		vec3 color = p.intensity * p.color ;

		//draw trail
		glBegin( GL_LINE_STRIP ) ;
		for( size_t j = 0 ; j < p.trail.size() ; ++j ) {
			glColor4f( color.r, color.b, color.g, (float)j/p.trail.size() ) ;
			glVertex3fv( value_ptr( p.trail[j] ) ) ;
		}
		glVertex3fv( value_ptr( p.pos ) ) ;
		glEnd() ;

		//draw particles
		glBegin( GL_POINTS ) ;
		glColor4f( color.r, color.b, color.g, 1.0-p.intensity ) ;
		glVertex3fv( value_ptr( particles[i].pos ) ) ;
		glEnd() ;

	}
	glPopAttrib() ;
}


void Display()
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT ) ;
	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity() ;

	DrawBackground() ;

	glTranslatef( 0, -10, -25 ) ;
	DrawParticles() ;

	glutSwapBuffers() ;
}


void Reshape( int w, int h )
{
	glViewport( 0, 0, w, h ) ;
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	gluPerspective( 45, (double)w/h, 0.1, 250 ) ;
	width = w, height = h ;
}


void GameLogic()
{
	static int cnt = 0 ;
	float dt = (float)time_interval * 0.001 ;
	current_time = glutGet( GLUT_ELAPSED_TIME ) ;

	//update particles
	for( size_t i = 0 ; i < particles.size() ; ++i )
	{
		//update position
		Particle& p = particles[i] ;
		p.pos += dt * p.vel ;
		p.vel += dt * gravity ;

		//update intensity
		p.intensity = std::max( 0.0f, (max_life - LifeTime(p))/max_life ) ;
		
		//update trail
		if( cnt > 5 ) {
			p.trail.push_back( p.pos ) ;
		}

		if( p.trail.size() > max_trail ) {
			p.trail.pop_front() ;
		}
		
		//kick old particle
		if( LifeTime(p) > max_life ) {
			particles.erase( particles.begin()+i ) ;		
		}

		//explode
		if( p.exploded == false && p.vel[1] < 0.5 ) {
			Explode(p) ;
			particles.erase( particles.begin()+i ) ;    //father died
		}
	}

	cnt > 5? cnt=0 : cnt++ ;
}


void Update( int dump )
{
	glutTimerFunc( time_interval, Update, 0 ) ;
	GameLogic() ;
	glutPostRedisplay() ;
}


void Launch( int dump )
{
	int rand_time = rand()%50+50 ;
	glutTimerFunc( rand_time, Launch, 0 ) ;
	particles.push_back( NewParticle() ) ;
	printf( "particles: %d\n", particles.size() ) ;
}


void KeyPress( unsigned char key, int x, int y )
{
	if( key == 27 ) {
		exit(0) ;
	}
	if( key == ' ' ) {
		particles.push_back( NewParticle() ) ;    //new particle
	}
}


int main( int argc, char** argv )
{
	glutInit( &argc, argv ) ;
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH ) ;
	glutInitWindowSize( width, height ) ;
	glutCreateWindow( "Firework Demo [Key: Space]" ) ;
	
	InitDemo() ;

	glutDisplayFunc( Display ) ;
	glutReshapeFunc( Reshape ) ;
	glutKeyboardFunc( KeyPress ) ;
	glutTimerFunc( time_interval, Launch, 0 ) ;    //launch firework
	glutTimerFunc( time_interval, Update, 0 ) ;    //update
	
	glutMainLoop() ;
	return 0 ;
}