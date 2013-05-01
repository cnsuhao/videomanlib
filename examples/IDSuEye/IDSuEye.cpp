#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include <iostream>
#include <stdio.h>
#include "VideoManControl.h"
#include "controllers/IuEyeCameraController.h"

using namespace std;
using namespace VideoMan;

/*
This is an example that shows how to use IDS uEye cameras
To use this example, VideoMan must be built with the directive VM_OGLRenderer
You also need to build the VMIDSuEye input
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
	VMInputIdentification *deviceList;
	int numDevices;

	videoMan.getAvailableDevices( "IDS_uEye_CAMERA", &deviceList, numDevices );
	cout << "Found " << numDevices << " IDS uEye cameras:" << endl;
	for ( int d = 0; d < numDevices; ++d )
		cout << "-" << deviceList[d].friendlyName << " with ID " << deviceList[d].uniqueName << endl;
	for ( int d = 0; d < numDevices; ++d )
	{
		format.showDlg = true;
		inputID = videoMan.addVideoInput( deviceList[d], &format );
		if ( inputID != -1 )
		{
			cout << " Initialized camera ID " << deviceList[d].uniqueName << endl;
			videoMan.setVerticalFlip( inputID, true );
			controller =  ((IuEyeCameraController*)videoMan.createController( 0, "uEye_CAMERA_CONTROLLER" ));
			double frameRate;
			controller->getFrameRate( frameRate );
			float exposure;
			controller->getShutterTime( exposure );
			//controller->setFrameRate( 8 );
			break;
		}
	}
	videoMan.freeAvailableDevicesList( &deviceList, numDevices );

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
	cout << "This is an example that shows how to use IDS uEye cameras with VMIDSuEye" << endl;
	cout << "=====================================================" << endl;
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan with IDS uEye cameras");
	glutHideWindow();

	if ( !InitializeVideoMan() )
	{
		cout << "Error intializing VideoMan" << endl;
		cout << "Pres Enter to exit" << endl;
		getchar();
		return -1;
	}

	glutShowWindow();
	glutReshapeFunc(glutResize );
    glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutDisplay);
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION);

	glutMainLoop();
	return 0;
}
