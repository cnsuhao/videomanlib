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
This is an example using VMImageSequence. 
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
also you need to build the input VMImageSequence
*/ 

VideoManControl videoMan;
bool fullScreened;
int videoInputID; //Index of the video input
char *imagesPath = 0;

void glutResize(int width, int height)
{
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( 0, 0, width, height );
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
		case 'n':
		case 'N':
		{
			//Release the frame causes 
			videoMan.releaseFrame( videoInputID );
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
    }
}

bool InitializeVideoMan()
{
	videoInputID = -1;
	if ( imagesPath )
	{
		VMInputFormat format;
		VMInputIdentification device;
		//Initialize one Image sequence input from a directory path
		device.fileName = imagesPath;
		device.identifier = "IMAGE_SEQUENCE";
		
		if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			//Show video info
			if ( device.fileName )
				cout << "Loading images from: " << device.fileName << endl;
			cout << "Resolution of the images: " << format.width << "X" << format.height << endl;
		}
	}
	return ( videoInputID != -1 );
}

void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );
	char *image = videoMan.getFrame( videoInputID );
	if ( image != NULL )
	{
		//Update the texture of the renderer
		videoMan.updateTexture( videoInputID );
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
	cout << "N->Next image" << endl;
	cout << "========" << endl;
}

int main(int argc, char** argv)
{
	cout << endl;
	cout << "This is an example using VMImageSequence input module for loading an image sequence" << endl;	
	cout << "Usage: exampleImageSequence.exe imagesPath(string)" << endl;
	cout << "Example: exampleImageSequence.exe c:\\mySequence" << endl;	
	if ( argc > 1 )
		imagesPath = argv[1];
	else
	{
		cout << "You must specify a path to a directory" << endl;
		return -1;
	}

	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan and image sequences");
	glutHideWindow();

	if ( !InitializeVideoMan() )
		return -1;

	glutShowWindow();
	glutReshapeFunc(glutResize);
    glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 

	fullScreened = false;
	showHelp();
    glutMainLoop();
	return 0;
}
