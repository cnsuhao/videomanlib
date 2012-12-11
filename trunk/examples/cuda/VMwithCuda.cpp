#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/glut.h>
#include <iostream>
#include <math.h>

#include "VideoManControl.h"
#include "FreeType.h"

#include "cudaEffect.cuh"

using namespace std;
using namespace VideoMan;

/*
This is an example using VideoMan with CUDA and OpenGL.
One input from a video file is initilized
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
*/

VideoManControl videoMan;
VMInputFormat format;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;
double videoLength;
bool paused = false; 
int inputID;
int processedID;
char *inputImg;
char *processedImg;

char *dirPath = 0;

float slider = 0.0f;
bool movingSlider = false;
int effect = 0;

freetype::font_data font;

void glutResize(int width, int height)
{
	screenLeft = 0;
	screenUp = 0;
	screenWidth = width;
	screenHeight = height;
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( screenLeft, screenUp, screenWidth, screenHeight );
}

void clear()
{
	freeGPUMem();
	delete processedImg;
	delete inputImg;
}

void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
		{
			clear();			
			exit(0);
		}
		case 'e':
		case 'E':
		{
			effect = ( effect + 1 ) % effectsNumber;
			break;
		}
		case 'z':
		case 'Z':
		{
			float x,y,zoom;
			videoMan.getRendererZoom( inputID, x, y, zoom );
			videoMan.setRendererZoom( inputID, x, y, zoom * 1.25f );
			break;
		}
		case 'r':
		case 'R':
		{
			float x,y,zoom;
			videoMan.getRendererZoom( inputID, x, y, zoom );
			videoMan.setRendererZoom( inputID, x, y, 1 );
			break;
		}
		case 'p':
		case 'P':
		{
			if ( paused )
				videoMan.playVideo( inputID );
			else
				videoMan.pauseVideo( inputID );
			paused = !paused;
		}
	}
}


void glutSpecialKeyboard(int value, int x, int y)
{
	switch (value)
    {
		case GLUT_KEY_F1:
		{
			if ( !fullScreened )
				glutFullScreen();
			else
			{
				glutPositionWindow( 0, 20 );
				glutReshapeWindow( 640, 480 );
			}
			fullScreened = !fullScreened;
			break;
		}		
    }
}

void glutMouseFunc( int button, int mouseState, int x, int y )
{
	float xf = (float)x;
	float yf = (float)y;
	//Transform from screen coordinates to image coordinates
	//Check if the point is inside the input region
	int input = videoMan.screenToImageCoords( xf, yf );
	
	if ( mouseState == GLUT_DOWN && button == GLUT_LEFT_BUTTON )
	{
		if ( input == inputID && fabs( xf - slider ) < 40 )
		{
			//If the click is over the slider
			movingSlider = true;
		}
	}
	else if ( mouseState == GLUT_UP && button == GLUT_LEFT_BUTTON )
	{
		movingSlider = false;
	}
}

void glutMouseMotionFunc(int x, int y)
{
	float xf = (float)x;
	float yf = (float)y;
	//Transform from screen coordinates to image coordinates
	//Check if the point is inside the input region
	int input = videoMan.screenToImageCoords( xf, yf );
	if ( movingSlider && input == inputID )
	{
		//Place the slider
		slider = xf;
	}
}

void InitializeOpenGL()
{
}


bool InitializeVideoMan()
{
	VMInputIdentification device;

	//Initialize one input from a video file
	inputID = -1;
	if ( dirPath )
	{	
		device.fileName = dirPath;
		#ifdef WIN32
			device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
		#elif linux
			device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui	
		#endif
		//play in real-time
		format.clock = true;
		format.dropFrames = true;
		format.renderAudio = true;		
		if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			if ( device.fileName )
				printf("Loaded video file: %s\n", device.fileName );
			printf("resolution: %d %d\n", format.width, format.height );
			//get the length of the video
			videoLength = videoMan.getLengthSeconds( inputID );
			printf("duration: %f seconds\n\n", videoLength );

			if ( format.nChannels != 3 || format.depth != 8 )
				cout << "The video must have 3 channels 8bit" << endl;

			videoMan.playVideo( inputID );		
		
			//Create a user input for the processed image
			device.identifier = "USER_INPUT";
			processedID = -1;
			processedID = videoMan.addVideoInput( device, &format );
			if ( processedID != -1 )
			{
				processedImg = new char[processedImg, format.width * format.height * format.nChannels];
				inputImg = new char[processedImg, format.width * format.height * format.nChannels];
				videoMan.setUserInputImage( processedID, processedImg );
			}
		}
		else
			return false;
	}
	
	slider = format.width * 0.5f;
	videoMan.deactivateVideoInput( processedID );
	
	return ( inputID != -1 && processedID != -1);
}

void printText( int vx, int vy, int vw, int vh, int x, int y, const char *text )
{
	glEnable( GL_BLEND );
	glViewport( vx, vy, vw, vh );
	glMatrixMode( GL_PROJECTION );	
	glLoadIdentity();
	glOrtho( 0, screenWidth, 0, screenHeight, -1.0, 1.0 ); 
	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();
	glColor4f( 1.0f, 1.0f, 1.0f, 0.9f);
	freetype::print( font, x, y, text );
	glDisable( GL_BLEND );
}

void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );

	//Get a new frame from input n
	char *image = videoMan.getFrame( inputID );
	if ( image != NULL )
	{
		videoMan.updateTexture( inputID );
		memcpy( inputImg, image, format.width * format.height * format.nChannels );
		videoMan.releaseFrame( inputID );
	}
	//Render the input with the original image
	videoMan.renderFrame( inputID );
	
	//Apply the CUDA kernel
	const int BPP = format.depth * format.nChannels / 8;
	memcpy( processedImg, inputImg, format.width * format.height * format.nChannels );
	executeCudaKernel( effect, (unsigned char*)processedImg, format.width, format.height, BPP );

	//Update the texture of processed input
	videoMan.updateTexture( processedID ); 

	printText( 0, 0, screenWidth, screenHeight, screenWidth - 100, screenHeight - 150, "Source" );
	
	//Render the uncovered part of the processed image
	videoMan.activate2DDrawingSetup( inputID );
	videoMan.activateTexture( processedID );
	float l, b, r, u;
	videoMan.getTextureCoords( processedID, l, b, r, u );
	glBegin(GL_QUADS);
		glTexCoord2f( l, b );
		glVertex2f( 0, 0 );
		glTexCoord2f( l, u );
		glVertex2f( 0, format.height );		
		glTexCoord2f( r * slider / format.width, u );
		glVertex2f( slider, format.height );
		glTexCoord2f( r * slider / format.width, b );
		glVertex2f( slider, 0 );		
	glEnd();

	//Render the slider
	glDisable( GL_TEXTURE_2D );
	glColor3f( 0.0f, 0.0f, 0.0f );
	glLineWidth( 4.0f );
	glBegin( GL_LINES );
		glVertex2f( slider, 0 );
		glVertex2f( slider, format.height );
	glEnd();
	glColor3f( 1.0f, 1.0f, 1.0f );
	glLineWidth( 1.0f );
	glBegin( GL_LINES );
		glVertex2f( slider, 0 );
		glVertex2f( slider, format.height );
	glEnd();

	//Check if the video file (input number 0) has reached the end	
	if ( videoMan.getPositionSeconds( inputID ) == videoLength )
		videoMan.goToFrame( 0, 0 ); //restart from the begining

	printText( 0, 0, slider, screenHeight, 0, screenHeight - 150, effectsname[effect] );	
	printText( 0, 0, screenWidth, screenHeight, 10, 200, "VideoMan Library (CUDA example) 2011 \nhttp://videomanlib.sourceforge.net \nJavier Barandiaran Martirena" );
	
    glutSwapBuffers();
}


void showHelp()
{
	printf("========\n");
	printf("keys:\n");	
	printf("Esc->Exit\n");
	printf("F1->Fullscreen\n");
	printf("E->Change the CUDA kernell\n");
	printf("Z->Zoom in\n");
	printf("R->Reset Zoom\n");
	printf("Left Mouse Button->Move the slider\n");
	printf("========\n");
}

int main(int argc, char** argv)
{
	cout << "This example initilizes one input from a video file" << endl;
	cout << "Usage: VMwithCuda.exe filePath(string)" << endl;	
	cout << "Example: VMwithCuda.exe c:\\video.avi" << endl;	
	cout << "=====================================================" << endl;
	if ( argc > 1 )
		dirPath = argv[1];
	else
		return 0;
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 1280, 720 );
    glutInit( &argc, argv );

    glutCreateWindow("VideoMan with NVidia CUDA");

    glutReshapeFunc(glutResize);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutMouseFunc( glutMouseFunc );
	glutMotionFunc( glutMouseMotionFunc );

    InitializeOpenGL();
	
	if ( !InitializeVideoMan() )
	{
		return 0;
	}
	
	allocGPUMem( format.width, format.height, format.nChannels * format.depth / 8 );
		
	fullScreened = false;

	//Load the font	
	try
	{
		font.init( "font.ttf", 16 );
	}
	catch(runtime_error e)
	{
		cerr << e.what() << endl;
		cerr << "Font file font.ttf not found. Copy that file to the working directory" << e.what() << endl;
		clear();
		return -1;
	}
	showHelp();

    glutMainLoop();

	clear();

	return 0;
}
