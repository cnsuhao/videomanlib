#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include <iostream>
#include "VideoManControl.h"
#include "controllers/IuEyeCameraController.h"

using namespace std;
using namespace VideoMan;

/*
This is an example that shows how to use cameras with VMHihgui
To use this example, VideoMan must be built with the directive VM_OGLRenderer
You also need to build the VMHihgui input
*/

VideoManControl videoMan;
IuEyeCameraController *controller;
int inputID; //Index of the video input

void glutResize(int width, int height)
{
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( 0, 0, width, height );
}

bool InitializeVideoMan()
{
	inputID = -1;
	VMInputFormat format;
	VMInputIdentification device;		
	
	device.identifier = "HIGHGUI_CAPTURE_DEVICE";
	device.uniqueName = "0";	
	format.SetFormat( 640, 480, 30, VM_RGB24, VM_RGB24 );
	inputID = videoMan.addVideoInput( device, &format );
	if ( inputID != -1 )
	{
		cout << " Initialized camera ID " << device.uniqueName << endl;
		videoMan.setVerticalFlip( inputID, true );
	}		

	return ( inputID != -1 );
}


void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );		

	if ( videoMan.getFrame( inputID ) )
	{
		videoMan.updateTexture( inputID );	
		videoMan.releaseFrame( inputID );
	}
	videoMan.renderFrame( inputID );

    glutSwapBuffers();
}

int main(int argc, char** argv)
{
	cout << endl << "=====================================================" << endl;	
	cout << "This is an example that shows how to use cameras with VMHighgui" << endl;	
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan with Highgui");
	glutHideWindow();

	if ( !InitializeVideoMan() )
		return -1;

	glutShowWindow();
	glutReshapeFunc(glutResize );  
    glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutDisplay);		
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);
    
	glutMainLoop();
	return 0;
}
