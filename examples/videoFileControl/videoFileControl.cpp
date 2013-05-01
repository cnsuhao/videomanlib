#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include <iostream>

#include "VideoManControl.h"
#include <stdlib.h>

using namespace std;
using namespace VideoMan;

/*
This is an example about controling video file playback. One video input is initialized form a video file
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
also you need to build the input VMDirectShow
*/ 


VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;
double videoLengthSeconds;
int videoLengthFrames;
int videoInputID; //Index of the video input
char *videoFile = 0;
bool paused = false;

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
}

void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
		{
			glutLeaveMainLoop();
			break;
		}
		case 'p':
		case 'P':
		{
			paused = !paused;
			if ( paused )
				videoMan.pauseVideo( videoInputID );
			else
				videoMan.playVideo( videoInputID );
			break;
		}
		case 'r':
		case 'R':
		{
			videoMan.goToFrame( videoInputID, 0 );		
			break;
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
		case GLUT_KEY_LEFT:
		{
			videoMan.goToMilisecond( videoInputID, videoMan.getPositionSeconds( videoInputID ) * 1000.0 - 1000.0 );
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			videoMan.goToMilisecond( videoInputID, videoMan.getPositionSeconds( videoInputID ) * 1000.0 + 1000.0 );
			break;
		}
		case GLUT_KEY_UP:
		{			
			videoMan.goToFrame( videoInputID, videoMan.getPositionFrames( videoInputID ) + 1 );
			break;
		}
		case GLUT_KEY_DOWN:
		{			
			videoMan.goToFrame( videoInputID, videoMan.getPositionFrames( videoInputID ) - 1 );			
			break;
		}
    }
}

bool InitializeVideoMan()
{
	videoInputID = -1;
	if ( videoFile )
	{
		VMInputFormat format;
		VMInputIdentification device;
		//Initialize one input from a video file
		device.fileName = videoFile;
		bool withHighgui = false;
		if ( videoMan.supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
			device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
		else if ( videoMan.supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
		{
			device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui	
			withHighgui = true;
		}
		//play in real-time
		format.clock = true;
		//Render the audio stream
		format.renderAudio = true;
		//Initialize the video file is the path
		if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			//get the length of the video
			videoLengthSeconds = videoMan.getLengthSeconds( videoInputID );
			videoLengthFrames = videoMan.getLengthFrames( videoInputID );
			
			//Show video info
			if ( device.fileName )
				cout << "Loaded video file: " << device.fileName << endl;
			cout << "resolution: " << format.width << "X" << format.height << endl;
			cout << "Video Length: " << videoLengthSeconds << " seconds and " << videoLengthFrames << " frames" << endl;						
			if ( withHighgui )
				videoMan.setVerticalFlip( videoInputID, true );
			videoMan.playVideo( videoInputID );
		}
	}	
	videoMan.activateAllVideoInputs();	
	return ( videoInputID != -1 );
}

void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );		
	if ( videoMan.getFrame( videoInputID ) != NULL )
	{
		//Update the texture of the renderer
		videoMan.updateTexture( videoInputID );
		//Release the frame
		videoMan.releaseFrame( videoInputID );
	}
	//render the image of input n in the screen
	videoMan.renderFrame( videoInputID );
    glutSwapBuffers();
}


void showHelp()
{
	cout << "========" << endl;
	cout << "keys:" << endl;
	cout << "Esc->Exit" << endl;
	cout << "F1->Fullscreen" << endl;
	cout << "P->Pause/Play video" << endl;
	cout << "R->Restart video" << endl;
	cout << "Arrows Left/Right->Backward/Forward (1 second)" << endl;
	cout << "Arrows Down/Up->Backward/Forward (1 frame)" << endl;
	cout << "========" << endl;
}

int main(int argc, char** argv)
{
	cout << "This is a simple example showing the control of video file playback" << endl;	
	cout << "Usage: videoFileControl.exe filePath(string)" << endl;
	cout << "Example: videoFileControl.exe c:\\video.avi" << endl;	
	cout << "=====================================================" << endl;

	if ( argc > 1 )
		videoFile = argv[1];
	else
	{
		showHelp();
		cout << "You must specify a path to a video file" << endl;
		cout << "Pres Enter to exit" << endl;
		getchar();
		return -1;
	}

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan video file playback control");
	glutHideWindow();

	if ( !InitializeVideoMan() )
	{
		showHelp();
		cout << "Error intializing VideoMan" << endl;
		cout << "Pres Enter to exit" << endl;
		getchar();
		return -1;
	}

	showHelp();
	glutShowWindow();
	glutReshapeFunc(glutResize);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 

	fullScreened = false;
    glutMainLoop();
	clear();
	return 0;
}
