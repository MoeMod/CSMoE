/*
 * This code was created by Jeff Molofee '99
 * (ported to Linux/SDL by Ti Leggett '01)
 *
 * If you've found this code useful, please let me know.
 *
 * Visit Jeff at http://nehe.gamedev.net/
 *
 * or for port-specific comments, questions, bugreports etc.
 * email to leggett@eecs.tulane.edu
 */

#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "wes_gl.h"
#include "wes_glu.h"
#include "context.h"
#include "wes_fragment.h"
#include "wes_state.h"


/* screen width, height, and bit depth */
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SCREEN_BPP     16

/* Set up some booleans */
#define TRUE  1
#define FALSE 0

int light = FALSE; /* Whether or not lighting is on */
int blend = FALSE; /* Whether or not blending is on */

GLfloat xrot;      /* X Rotation */
GLfloat yrot;      /* Y Rotation */
GLfloat xspeed;    /* X Rotation Speed */
GLfloat yspeed;    /* Y Rotation Speed */
GLfloat z = -5.0f; /* Depth Into The Screen */
GLfloat zspeed = 0.0;

/* Ambient Light Values ( NEW ) */
GLfloat LightAmbient[]  = { 0.5f, 0.5f, 0.5f, 1.0f };
/* Diffuse Light Values ( NEW ) */
GLfloat LightDiffuse[]  = { 1.0f, 1.0f, 1.0f, 1.0f };
/* Light Position ( NEW ) */
GLfloat LightPosition[] = { 0.0f, 0.0f, 2.0f, 1.0f };

GLuint filter;     /* Which Filter To Use */
GLuint texture[3]; /* Storage for 3 textures */

/* function to release/destroy our resources and restoring the old desktop */
void Quit( int returnCode )
{
    /* clean up the window */
    SDL_Quit( );
    wes_destroy();
    wes_context_close();

    /* and exit appropriately */
    exit( returnCode );
}

/* function to load in bitmap as a GL texture */
int LoadGLTextures( )
{
    /* Status indicator */
    int Status = FALSE;

    /* Create storage space for the texture */
    SDL_Surface *TextureImage[1];

    /* Load The Bitmap, Check For Errors, If Bitmap's Not Found Quit */
    if ( ( TextureImage[0] = SDL_LoadBMP( "data/glass.bmp" ) ) )
        {

	    /* Set the status to true */
	    Status = TRUE;

	    /* Create The Texture */
	    glGenTextures( 3, &texture[0] );

	    /* Load in texture 1 */
	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, texture[0] );

	    /* Generate The Texture */
	    glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[0]->w, TextureImage[0]->h, 0, GL_BGR, GL_UNSIGNED_BYTE, TextureImage[0]->pixels );

	    /* Nearest Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	    /* Load in texture 2 */
	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, texture[1] );

	    /* Linear Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	    /* Generate The Texture */
	    glTexImage2D( GL_TEXTURE_2D, 0, 3, TextureImage[0]->w,  TextureImage[0]->h, 0, GL_BGR, GL_UNSIGNED_BYTE, TextureImage[0]->pixels );

	    /* Load in texture 3 */
	    /* Typical Texture Generation Using Data From The Bitmap */
	    glBindTexture( GL_TEXTURE_2D, texture[2] );

	    /* Mipmapped Filtering */
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
	    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	    /* Generate The MipMapped Texture ( NEW ) */
	    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TextureImage[0]->w, TextureImage[0]->h, GL_BGR, GL_UNSIGNED_BYTE, TextureImage[0]->pixels );
        }

    /* Free up any memory we may have used */
    if ( TextureImage[0] )
	    SDL_FreeSurface( TextureImage[0] );

    return Status;
}

/* function to reset our viewport after a window resize */
int resizeWindow( int width, int height )
{
    /* Height / width ration */
    GLfloat ratio;

    /* Protect against a divide by zero */
    if ( height == 0 )
	height = 1;

    ratio = ( GLfloat )width / ( GLfloat )height;

    /* Setup our viewport. */
    glViewport( 0, 0, ( GLint )width, ( GLint )height );

    /* change to the projection matrix and set our viewing volume. */
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity( );

    /* Set our perspective */
    gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

    /* Make sure we're chaning the model view and not the projection */
    glMatrixMode( GL_MODELVIEW );

    /* Reset The View */
    glLoadIdentity( );

    return( TRUE );
}

/* function to handle key press events */
void handleKeyPress( SDL_keysym *keysym )
{
    switch ( keysym->sym )
	{
	case SDLK_ESCAPE:
	    /* ESC key was pressed */
	    Quit( 0 );
	    break;
	case SDLK_b:
	    /* 'b' key was pressed
	     * this toggles blending
	     */
	    blend = !blend;
	    if ( blend )
		{
		    glEnable( GL_BLEND );
		    glDisable( GL_DEPTH_TEST );
		}
	    else
		{
		    glDisable( GL_BLEND );
		    glEnable( GL_DEPTH_TEST );
		}
	    break;
	case SDLK_f:
	    /* 'f' key was pressed
	     * this pages through the different filters
	     */
	    filter = ( ++filter ) % 3;
	    break;
	case SDLK_l:
	    /* 'l' key was pressed
	     * this toggles the light
	     */
	    light = !light;
	    if ( !light )
		glDisable( GL_LIGHTING );
	    else
		glEnable( GL_LIGHTING );
	    break;
	case SDLK_PAGEUP:
	    /* PageUp key was pressed
	     * this zooms into the scene
	     */
	    zspeed -= 0.01f;
	    break;
	case SDLK_PAGEDOWN:
	    /* PageDown key was pressed
	     * this zooms out of the scene
	     */
	    zspeed += 0.01f;
	    break;
	case SDLK_UP:
	    /* Up arrow key was pressed
	     * this affects the x rotation
	     */
	    xspeed -= 0.01f;
	    break;
	case SDLK_DOWN:
	    /* Down arrow key was pressed
	     * this affects the x rotation
	     */
	    xspeed += 0.01f;
	    break;
	case SDLK_RIGHT:
	    /* Right arrow key was pressed
	     * this affects the y rotation
	     */
	    yspeed += 0.01f;
	    break;
	case SDLK_LEFT:
	    /* Left arrow key was pressed
	     * this affects the y rotation
	     */
	    yspeed -= 0.01f;
	    break;
	case SDLK_F1:
	    /* 'f' key was pressed
	     * this toggles fullscreen mode
	     */
	    break;
    case SDLK_SPACE:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
        break;

    case SDLK_c:
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
        break;

	default:
	    break;
	}

    return;
}

/* general OpenGL initialization function */
int initGL( GLvoid )
{

    /* Load in the texture */
    if ( !LoadGLTextures( ) )
        return FALSE;

    /* Enable Texture Mapping ( NEW ) */
    glEnable( GL_TEXTURE_2D );

    /* Enable smooth shading */
    //glShadeModel( GL_SMOOTH );

    /* Set the background black */
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    /* Depth buffer setup */
    glClearDepth( 1.0f );

    /* Enables Depth Testing */
    glEnable( GL_DEPTH_TEST );

    /* The Type Of Depth Test To Do */
    glDepthFunc( GL_LEQUAL );

    /* Really Nice Perspective Calculations */
    //glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

    /* Setup The Ambient Light */
    glLightfv( GL_LIGHT1, GL_AMBIENT, LightAmbient );

    /* Setup The Diffuse Light */
    glLightfv( GL_LIGHT1, GL_DIFFUSE, LightDiffuse );

    /* Position The Light */
    glLightfv( GL_LIGHT1, GL_POSITION, LightPosition );

    /* Enable Light One */
    glEnable( GL_LIGHT1 );

    /* Full Brightness, 50% Alpha ( NEW ) */
    glColor4f( 1.0f, 1.0f, 1.0f, 0.5f);

    /* Blending Function For Translucency Based On Source Alpha Value  */
    glBlendFunc( GL_SRC_ALPHA, GL_ONE );

    return( TRUE );
}

/* Here goes our drawing code */
int drawGLScene( GLvoid )
{
    /* These are to calculate our fps */
    static GLint T0     = 0;
    static GLint Frames = 0;

    /* Clear The Screen And The Depth Buffer */
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    /* Reset the view */
    glLoadIdentity( );

    /* Translate Into/Out Of The Screen By z */
    glTranslatef( 0.0f, 0.0f, z );

    glRotatef( xrot, 1.0f, 0.0f, 0.0f); /* Rotate On The X Axis By xrot */
    glRotatef( yrot, 0.0f, 1.0f, 0.0f); /* Rotate On The Y Axis By yrot */

    /* Select A Texture Based On filter */
    glColor4f(1,1,1,0.5);
    glBindTexture( GL_TEXTURE_2D, texture[filter] );

    /* Start Drawing Quads */
    glBegin( GL_QUADS );
      /* Front Face */
      /* Normal Pointing Towards Viewer */
      glNormal3f( 0.0f, 0.0f, 1.0f );
      /* Point 1 (Front) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );

      /* Point 2 (Front) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );

      /* Point 3 (Front) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
      /* Point 4 (Front) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );

      /* Back Face */
      /* Normal Pointing Away From Viewer */
      glNormal3f( 0.0f, 0.0f, -1.0f);
      /* Point 1 (Back) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Point 2 (Back) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
      /* Point 3 (Back) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );
      /* Point 4 (Back) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );

      /* Top Face */
      /* Normal Pointing Up */
      glNormal3f( 0.0f, 1.0f, 0.0f );
      /* Point 1 (Top) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
      /* Point 2 (Top) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );
      /* Point 3 (Top) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f(  1.0f,  1.0f,  1.0f );
      /* Point 4 (Top) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f(  1.0f,  1.0f, -1.0f );

      /* Bottom Face */
      /* Normal Pointing Down */
      glNormal3f( 0.0f, -1.0f, 0.0f );
      /* Point 1 (Bottom) */
    glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );
      /* Point 2 (Bottom) */
        glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f(  1.0f, -1.0f, -1.0f );
      /* Point 3 (Bottom) */
        glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f(  1.0f, -1.0f,  1.0f );
      /* Point 4 (Bottom) */
            glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );

      /* Right face */
      /* Normal Pointing Right */
      glNormal3f( 1.0f, 0.0f, 0.0f);

      /* Point 1 (Right) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( 1.0f, -1.0f, -1.0f );

      /* Point 2 (Right) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( 1.0f,  1.0f, -1.0f );

      /* Point 3 (Right) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( 1.0f,  1.0f,  1.0f );

      /* Point 4 (Right) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( 1.0f, -1.0f,  1.0f );

      /* Left Face*/
      /* Normal Pointing Left */
      glNormal3f( -1.0f, 0.0f, 0.0f );
      /* Point 1 (Left) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 0.0);
      glTexCoord2f( 1.0f, 0.0f ); glVertex3f( -1.0f, -1.0f, -1.0f );

      /* Point 2 (Left) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 0.0);
      glTexCoord2f( 0.0f, 0.0f ); glVertex3f( -1.0f, -1.0f,  1.0f );

      /* Point 3 (Left) */
      glMultiTexCoord2f(GL_TEXTURE1, 0.0, 1.0);
      glTexCoord2f( 0.0f, 1.0f ); glVertex3f( -1.0f,  1.0f,  1.0f );

      /* Point 4 (Left) */
      glMultiTexCoord2f(GL_TEXTURE1, 1.0, 1.0);
      glTexCoord2f( 1.0f, 1.0f ); glVertex3f( -1.0f,  1.0f, -1.0f );
    glEnd();

    /* Draw it to the screen */
    //SDL_GL_SwapBuffers( );
    wes_context_update();

    /* Gather our frames per second */
    Frames++;
    {
	GLint t = SDL_GetTicks();
	if (t - T0 >= 500) {
	    GLfloat seconds = (t - T0) / 1000.0;
	    GLfloat fps = Frames / seconds;
	    printf("%d frames in %g seconds = %g FPS\n", Frames, seconds, fps);
	    T0 = t;
	    Frames = 0;
	}
    }

    z += zspeed;
    xrot += xspeed; /* Add xspeed To xrot */
    yrot += yspeed; /* Add yspeed To yrot */

    return( TRUE );
}

int main( int argc, char **argv )
{

    /* main loop variable */
    int done = FALSE;
    /* used to collect events */
    SDL_Event event;

    /* whether or not the window is active */
    int isActive = TRUE;

    /* Get Context  */
    wes_context_open(SCREEN_WIDTH, SCREEN_HEIGHT, 0);

    if (argc == 1){
        freopen( "CON", "w", stdout );
        freopen( "CON", "w", stderr );
    } else {
        if (!strcmp(argv[1], "-log")){
            freopen("stdout.txt", "w", stdout );
            freopen("stderr.txt", "w", stderr );
        }
    }
    printf("Context OPen");

    wes_init("libGLESv2.dll");

    /* initialize OpenGL */
    initGL( );

    glEnable(GL_ALPHA_TEST);
    glAlphaFunc(GL_NOTEQUAL, 0.75);

    glActiveTexture(GL_TEXTURE0);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PREVIOUS);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_ALPHA);

    glActiveTexture(GL_TEXTURE1);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_RGB, GL_PRIMARY_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_RGB, GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_COLOR);
    glTexEnvf(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, GL_PREVIOUS);
    glTexEnvf(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, GL_TEXTURE);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_ALPHA);
    glTexEnvf(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_ALPHA);
    glBindTexture( GL_TEXTURE_2D, texture[0]);
    glActiveTexture(GL_TEXTURE0);



    SDL_EnableKeyRepeat( 100, SDL_DEFAULT_REPEAT_INTERVAL ) ;

    /* resize the initial window */
    resizeWindow( SCREEN_WIDTH, SCREEN_HEIGHT );

    /* wait for events */
    while ( !done )
	{
	    /* handle the events in the queue */
	    while ( SDL_PollEvent( &event ) )
		{
		    switch( event.type )
			{
			case SDL_ACTIVEEVENT:
			    if ( event.active.gain == 0 )
				isActive = FALSE;
			    else
				isActive = TRUE;
			    break;
			case SDL_VIDEORESIZE:
			    break;
			case SDL_KEYDOWN:
			    handleKeyPress( &event.key.keysym );
			    break;
			case SDL_QUIT:
			    done = TRUE;
			    break;
			default:
			    break;
			}
		}

	    /* draw the scene */
	    if ( isActive )
            drawGLScene( );
	}

    /* clean ourselves up and exit */
    Quit( 0 );

    /* Should never get here */
    return( 0 );
}
