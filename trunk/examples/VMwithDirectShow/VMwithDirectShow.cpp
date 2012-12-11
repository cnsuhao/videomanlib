#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>

#include "VideoManControl.h"

using namespace std;
using namespace VideoMan;

/*
This is an example using VideoMan with DirectShow and OpenGL.
Two inputs are initilized using DirectShow: one video file and one camera
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
also you need to build the input VMDirectShow
*/

VideoManControl videoMan; //VideoMan control object
int screenLeft, screenUp, screenWidth, screenHeight; //Window's size
bool fullScreened;  //To nkow if the application is in fullscreen mode
double videoLength; //Length of the initialized video file
bool activated; //To know if the first input is activated or not
std::vector< int > videoInputIDs; //List of indexes of the initialized inputs
int videoFileInputID;

char *dirPath = 0;

void glutResize(int width, int height)
{
	screenLeft = 0;
	screenUp = 0;
	screenWidth = width;
	screenHeight = height;
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( screenLeft, screenUp, screenWidth, screenHeight );
}


void glutKeyboard(unsigned char key, int x, int y)
{
	switch (key)
	{
		case 27:
		{
			exit(0);
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
		case GLUT_KEY_F2:
		{
			if ( activated )			
				videoMan.deactivateVideoInput( 0 );
			else
				videoMan.activateVideoInput( 0 );
			activated = videoMan.isActivated(0);
			break;
		}		
    }
}


void InitializeOpenGL()
{
}


bool InitializeVideoMan()
{
	VMInputFormat format;	
	VMInputIdentification device;

	//Initialize one input from a video file
	int inputID;
 	videoFileInputID = -1;
	if ( dirPath )
	{	
		device.fileName = dirPath;
		device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
		//play in real-time using the framrate of the video
		format.clock = true;
		format.dropFrames = true;
		format.renderAudio = true;		
		if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			videoFileInputID = inputID;
			videoInputIDs.push_back( inputID );
			if ( device.fileName )
				printf("Loaded video file: %s\n", device.fileName );
			printf("resolution: %d %d\n", format.width, format.height );

			//get the length of the video
			videoLength = videoMan.getLengthSeconds( inputID );
			printf("duration: %f seconds\n\n", videoLength );

			videoMan.playVideo( inputID );
		}	
	}

	//Initialize one input from a camera	
	VMInputIdentification *list;
	int numDevices;
	videoMan.getAvailableDevices( "DSHOW_CAPTURE_DEVICE", &list, numDevices ); //list all the available devices
	if ( numDevices > 0 )
	{
		device = list[0]; //take the first
		//Show dialog to select the format
		format.showDlg = true;	
		if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			videoInputIDs.push_back( inputID );
			//videoMan.showPropertyPage( inputID );
			videoMan.getFormat( inputID, format );
			printf("Initilized camera: %s %s\n", device.friendlyName ? device.friendlyName : "", device.uniqueName ? device.uniqueName : "" );						
			printf("resolution: %d %d\n", format.width, format.height );
			printf("FPS: %f\n\n", format.fps );
		}
	}
	else
		printf("There is no available camera\n");	
	videoMan.freeAvailableDevicesList(  &list, numDevices );
	
	//We want to display all the intialized video inputs
	videoMan.activateAllVideoInputs();	
	activated = true;

	return ( videoInputIDs.size() > 0);
}


void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );
	//For each initialized inputs
	for ( size_t n=0; n < videoInputIDs.size(); n++ )
	{
		//Get a new frame from input n
		char *image = videoMan.getFrame( videoInputIDs[n] );
		if ( image != NULL )
		{
			//Update the texture of the renderer
			videoMan.updateTexture( videoInputIDs[n] ); 
            
			/*
				Process the image...
			*/

			//Release the frame
			videoMan.releaseFrame( videoInputIDs[n] );
		}
		//render the image of input n in the screen
		videoMan.renderFrame( videoInputIDs[n] ); 
	}

	//Check if the video file (input number 0) has reached the end	
	if ( videoFileInputID != -1 && videoMan.getPositionSeconds( videoFileInputID ) == videoLength )
		videoMan.goToFrame( videoFileInputID, 0 ); //restart from the begining

	glFlush();
    glutSwapBuffers();
}


void showHelp()
{
	printf("========\n");
	printf("keys:\n");	
	printf("Esc->Exit\n");
	printf("F1->Fullscreen\n");
	printf("F2->Show Video File y/n\n");
	printf("========\n");
}

int main(int argc, char** argv)
{
	cout << "This example initilizes two inputs: one video file and one camera" << endl;
	cout << "Usage: VMwithDirectShow.exe filePath(string)" << endl;	
	cout << "Example: VMwithDirectShow.exe c:\\video.avi" << endl;
	cout << "The camera is automatically detected" << endl;		
	cout << "=====================================================" << endl;
	if ( argc > 1 )
		dirPath = argv[1];
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );

    glutCreateWindow("VideoMan with DirectShow");

    glutReshapeFunc(glutResize);
    glutDisplayFunc(glutDisplay);
    glutIdleFunc(glutDisplay);
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);

    InitializeOpenGL();
	
	if ( !InitializeVideoMan() )
	{
		return 0;
	}
	
	fullScreened = false;

	showHelp();

    glutMainLoop();

	return 0;
}