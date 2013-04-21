#include <windows.h>
#include <GL/freeglut.h>
#include <iostream>
#include <vector>

#include "VideoManControl.h"
#include "controllers/IDSPointGreyController.h"

using namespace std;
using namespace VideoMan;

/*
This is an example using VideoMan with DirectShow and OpenGL.
This examples also show how to use the PointGrey controller to change camera properties
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
also you need to build the input VMDirectShow
*/

VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;
int  inputID;

IDSPointGreyController *controller;

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
	videoMan.deleteController( (VideoManPrivate::VideoManInputController**) &controller );
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
	VMInputFormat format;

	//Initialize one input from a camera	
	VMInputIdentification *list;
	int numDevices;
	videoMan.getAvailableDevices( "DSHOW_CAPTURE_DEVICE", &list, numDevices ); //list all the available devices
	for ( int d = 0; d < numDevices; ++d )	
	{
		//Show dialog to select the format
		format.showDlg = true;	
		if ( ( inputID = videoMan.addVideoInput( list[d], &format ) ) != -1 )
		{
			videoMan.showPropertyPage( inputID );
			videoMan.getFormat( inputID, format );
			if ( list[d].friendlyName )
				printf("Initilized camera: %s\n", list[d].friendlyName );
			printf("resolution: %d %d\n", format.width, format.height );
			printf("FPS: %f\n\n", format.fps );
			controller = (IDSPointGreyController*)videoMan.createController( inputID, "DSHOW_POINTGREY_CONTROLLER" );
			if ( controller )
			{
			}
			break;
		}
	}	
	videoMan.freeAvailableDevicesList(  &list, numDevices );
	if ( inputID == -1 )
	{
		printf("There is no available camera\n");	
		return false;
	}

	return true;
}


void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );
	//For each initialized inputs	
	
	char *image = videoMan.getFrame( inputID );
	if ( image != NULL )
	{
		//Update the texture of the renderer
		videoMan.updateTexture( inputID );         		

		//Release the frame
		videoMan.releaseFrame( inputID );
	}
	//render the image of input n in the screen
	videoMan.renderFrame( inputID ); 	

	glFlush();
    glutSwapBuffers();
}


void showHelp()
{
	printf("========\n");
	printf("keys:\n");	
	printf("Esc->Exit\n");
	printf("F1->Fullscreen\n");
	printf("S->Change the input channel (framegrabbes)\n");
	printf("G->Turn on/off the Gain automatic control\n");
	printf("========\n");
}

int main(int argc, char** argv)
{
	cout << "This example initilizes one camera" << endl;
	cout << "=====================================================" << endl;
	
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan with DirectShow");
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
	clear();
	return 0;
}