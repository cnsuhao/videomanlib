#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/glut.h>
#include <iostream>
#include "VideoManControl.h"

using namespace std;
using namespace VideoMan;

/*
This is an example that shows how to use PointGrey cameras 
To use this example, VideoMan must be built with the directive VM_OGLRenderer
You also need to build the VMPointGrey2 input
*/

VideoManControl videoMan;

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

	videoMan.getAvailableDevices( "PGR_CAMERA2", &deviceList, numDevices );
	cout << "Found " << numDevices << " PointGrey cameras:" << endl;
	for ( int d = 0; d < numDevices; ++d )
		cout << "-" << deviceList[d].friendlyName << " with serial " << deviceList[d].uniqueName << endl;
	for ( int d = 0; d < numDevices; ++d )
	{
		format.showDlg = true;
		inputID = videoMan.addVideoInput( deviceList[d], &format );
		if ( inputID != -1 )
		{
			cout << " Initialized camera " << deviceList[d].friendlyName << " " << deviceList[d].uniqueName << endl;
			videoMan.setVerticalFlip( inputID, true );
			break;
		}
		++d;
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
	cout << endl << "=====================================================" << endl;	
	cout << "This is an example that shows how to use PointGrey cameras " << endl;	
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan with PointGrey");
	glutReshapeFunc(glutResize );  
    glutDisplayFunc(glutDisplay);
	glutIdleFunc(glutDisplay);
 
	if ( !InitializeVideoMan() )
		return -1; 
    
	glutMainLoop();
	return 0;
}
