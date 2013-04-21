#ifdef WIN32
	#include <windows.h>
#endif
#include <vector>
#include <GL/freeglut.h>
#include <iostream>
#include "cv.h"
#include "cxcore.h"

#include "VideoManControl.h"

using namespace std;
using namespace VideoMan;

/*
This is a simple example using OpenCV. One video input is initialized and it is processed using openCV
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
also you need to build the input VMDirectShow
*/ 

VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;
double videoLength;
int videoInputID; //Index of the video input
std::vector< int > userInputIDs; //Indexes of the userinputs for showing the processed images

IplImage *inputCopy;	//Copy of the input image
IplImage *processedImages[3]; //The processed images
IplImage *edges;
IplImage *gray;

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

void clear()
{
	videoMan.deleteInputs();
	for ( int i = 0; i< 3; ++i )
		cvReleaseImage( &processedImages[i] );
	cvReleaseImage( &inputCopy );
	cvReleaseImage( &edges );
	cvReleaseImage( &gray );	
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
		case GLUT_KEY_F2:
		{
			if ( videoMan.isActivated( userInputIDs[0] )	)	
				videoMan.deactivateVideoInput( userInputIDs[0] );
			else
				videoMan.activateVideoInput( userInputIDs[0] );			
			break;
		}
		case GLUT_KEY_F3:
		{
			if ( videoMan.isActivated( userInputIDs[1] )	)	
				videoMan.deactivateVideoInput( userInputIDs[1] );
			else
				videoMan.activateVideoInput( userInputIDs[1] );			
			break;
		}		
		case GLUT_KEY_F4:
		{
			if ( videoMan.isActivated( userInputIDs[2] )	)	
				videoMan.deactivateVideoInput( userInputIDs[2] );
			else
				videoMan.activateVideoInput( userInputIDs[2] );			
			break;
		}
		case GLUT_KEY_F5:
		{			
			static int mode = 0;
			mode = (mode + 1 ) %9;
			videoMan.changeVisualizationMode( mode );

			break;
		}	
		case GLUT_KEY_F6:
		{			
			static int main = 0;
			main = (main + 1 ) %videoMan.getNumberOfInputs() ;
			videoMan.changeMainVisualizationInput( main );
			break;
		}	
    }
}

void glutDisplay(void)
{
	//Clear the opengl window
	glClear( GL_COLOR_BUFFER_BIT );
	
	char *image = videoMan.getFrame( videoInputID );
	if ( image != NULL )
	{
		memcpy( inputCopy->imageData, (const char *)image, inputCopy->imageSize );	
	
		//Update the texture of the renderer
		videoMan.updateTexture( videoInputID, inputCopy->imageData ); 
	       
		//cvSetImageData( inputHeader, image, inputHeader->widthStep );
		// Process the images...
		
		//Convert to grayscale
		if ( inputCopy->nChannels != 1 )
			cvCvtColor( inputCopy, gray, CV_RGB2GRAY );
		else
			cvCopy( inputCopy, gray );
		
		//process 0
		if ( videoMan.isActivated( userInputIDs[0] ) )
		{
			cvNot( inputCopy, processedImages[0] );
			videoMan.updateTexture( userInputIDs[0] );
		}

		//process 1 
		if ( videoMan.isActivated( userInputIDs[1] ) && inputCopy->depth == 8 )
		{
			cvSmooth( inputCopy, processedImages[1], CV_BLUR, 7, 7 );
			videoMan.updateTexture( userInputIDs[1] );				
		}

		//process 2
		if ( videoMan.isActivated( userInputIDs[2]) && inputCopy->depth == 8 )
		{
			cvZero( processedImages[2] );
			cvCanny( gray, edges, 50, 60 );		
			cvCopy( inputCopy, processedImages[2],edges );		
			videoMan.updateTexture( userInputIDs[2] );	
		}

		//Release the frame
		videoMan.releaseFrame( videoInputID );
	}

	//render the image of input n in the screen
	videoMan.renderFrame( videoInputID ); 	
	for ( int i = 0; i < 3; ++i )
		videoMan.renderFrame( userInputIDs[i] );

	//Check if the video file (input number 0) has reached the end	
	if ( videoMan.getPositionSeconds(videoInputID) == videoLength )
		videoMan.goToFrame( videoInputID, 0 ); //restart from the begining

    glutSwapBuffers();
}

bool InitializeVideoMan()
{
	VMInputFormat format;	
	VMInputIdentification device;

	if ( dirPath )
	{
		//Initialize one input from a video file
		//device.fileName = dirPath.c_str();
		device.fileName = dirPath;
		if ( videoMan.supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
			device.identifier = "DSHOW_VIDEO_FILE"; //using directshow	
		else if ( videoMan.supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
			device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui	
		//play in real-time
		format.clock = true;
		format.renderAudio = true;
		//Initialize the video file is the path 
		if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
		{
			if ( device.fileName )
				cout << "Loaded video file: " << device.fileName << endl;
			cout << "resolution: " << format.width << " " << format.height << endl;

			//get the length of the video
			videoLength = videoMan.getLengthSeconds( videoInputID );
			cout << "duration: " << videoLength << " seconds" << endl;
			
			videoMan.playVideo( videoInputID );
		}
	}
	else
	{
		//Initialize one input from a camera	
		//std::vector<VMInputIdentification> list;
		//videoMan.getAvailableDevices( list ); //list all the available devices
		VMInputIdentification *list;
		int numDevices;
		videoMan.getAvailableDevices( &list, numDevices ); //list all the available devices
		if ( numDevices == 0 )			
		{
			cout << "There is no available camera\n" << endl;
			videoMan.freeAvailableDevicesList(  &list, numDevices );
			return false;
		}
		//Intialize on of the devices
		int d = 0;
		videoInputID = -1;
		while ( d < numDevices && videoInputID == -1 )
		{			
			device = list[d];
			//Show dialog to select the format
			format.showDlg = true;
			format.SetFormat( 640, 480, 30, VM_RGB24, VM_RGB24 );
			if ( ( videoInputID = videoMan.addVideoInput( device, &format ) ) != -1 )
			{
				videoMan.showPropertyPage( videoInputID );
				videoMan.getFormat( videoInputID, format );
				if ( device.friendlyName )
					cout << "Initilized camera: " << device.friendlyName << endl;
				cout << "resolution: " <<  format.width << " " << format.height << endl;
				cout << "FPS: " << format.fps << endl;
			}
			++d;
		}
		videoMan.freeAvailableDevicesList(  &list, numDevices );
	}

	if ( videoInputID != -1 )
	{		
		inputCopy = cvCreateImage( cvSize( format.width, format.height), format.depth, format.nChannels );

		//Initialize the user inputs for showing the processed images
		device.identifier = "USER_INPUT";
		int inputID;
		for ( int i = 0; i < 3; ++i )
		{
			if ( ( inputID = videoMan.addVideoInput( device, &format ) ) != -1 )
			{
				userInputIDs.push_back( inputID );
				processedImages[i] = cvCreateImage( cvSize( format.width, format.height), format.depth, format.nChannels );
				videoMan.setUserInputImage( inputID, processedImages[i]->imageData );
			}
		}
		edges = cvCreateImage( cvSize( format.width, format.height), format.depth, 1 );
		gray = cvCreateImage( cvSize( format.width, format.height), format.depth, 1 );
	}
	
	//We want to display all the intialized video inputs
	videoMan.activateAllVideoInputs();	

	return ( videoInputID != -1 );
}

void showHelp()
{
	printf("========\n");
	printf("keys:\n");	
	printf("Esc->Exit\n");
	printf("F1->Fullscreen\n");
	printf("F2-F4->Show Processed Images y/n\n");
	printf("F5->Change Visualization Mode\n");
	printf("F6->Change Main Visualization Input\n");
	printf("========\n");
}

int main(int argc, char** argv)
{
	cout << "This is a simple example using OpenCV. One video input is initialized and it is processed using openCV" << endl;	
	cout << "Usage: opencvSimple.exe filePath(string)" << endl;
	cout << "Example: opencvSimple.exe c:\\video.avi" << endl;
	cout << "If you don't specify a filepath, a camera will be initialized" << endl;
	if ( argc > 1 )
		dirPath = argv[1];
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
    glutInitWindowPosition( 0, 0 );
    glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan-OpenCV Simple");
	glutHideWindow();   
	
	if ( !InitializeVideoMan() )
		return 0;

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
