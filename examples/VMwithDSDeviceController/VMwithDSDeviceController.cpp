#include <windows.h>
#include <GL/glut.h>
#include <iostream>
#include <vector>

#include "VideoManControl.h"
#include "controllers/ICaptureDeviceDSController.h"

using namespace std;
using namespace VideoMan;

/*
This is an example using VideoMan with DirectShow and OpenGL.
This examples also show how to use the camera controller to change camera properties
To use this example, VideoMan must be built with the directive VM_OGLRenderer, 
also you need to build the input VMDirectShow
*/

VideoManControl videoMan;
int screenLeft, screenUp, screenWidth, screenHeight;
bool fullScreened;
int  inputID;

ICaptureDeviceDSController *controller;
int numInputchannels = 0;
int channel = 0;

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
	videoMan.deleteController( (VideoManPrivate::VideoManInputController**)&controller );
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
		case 'c':
		case 'C':
		{
			if ( controller && numInputchannels > 0 )
			{
				channel = ( channel + 1 ) % numInputchannels;
				controller->setInputChannel( channel );
				
			}
			break;
		}
		case 'g':
		case 'G':
		{
			long value;
			bool flagAuto;
			if ( controller && controller->getImageProperty( ICaptureDeviceDSController::Gain, value, flagAuto ) )
			{
				//Gain is supported
				if ( flagAuto )
					controller->setImageProperty( ICaptureDeviceDSController::Gain, value, false );//Turn Manual
				else
					controller->setImageProperty( ICaptureDeviceDSController::Gain, value, true );//Turn Auto
			}
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


void InitializeOpenGL()
{
}

void printCameraControl( ICaptureDeviceDSController::CameraControl identifier, std::string name )
{
	long min, max, step, value;
	bool flagAuto;
	if ( controller->getCameraControlRange( identifier, min, max, step, value, flagAuto ) )
	{
		cout << name << " Range: " << min << "-" << max << " step: " << step << " default: " << value;
		cout << ( flagAuto ? " Auto" : " Manual" ) << endl;
	}
	if ( controller->getCameraControl( identifier, value, flagAuto ) )	
		cout << "Current " << name <<": " << value << ( flagAuto ? " Auto" : " Manual" )  << endl;
}

void printImageProperty( ICaptureDeviceDSController::ImageProperty identifier, std::string name )
{
	long min, max, step, value;
	bool flagAuto;
	if ( controller->getImagePropertyRange( identifier, min, max, step, value, flagAuto ) )
	{
		cout << name << " Range: " << min << "-" << max << " step: " << step << " default: " << value;
		cout << ( flagAuto ? " Auto" : " Manual" ) << endl;
	}
	if ( controller->getImageProperty( identifier, value, flagAuto ) )
		cout << "Current " << name << ": " << value << ( flagAuto ? " Auto" : " Manual" )  << endl;
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
			controller = (ICaptureDeviceDSController*)videoMan.createController( inputID, "DSHOW_CAPTURE_DEVICE_CONTROLLER" );
			if ( controller )
			{
				//These dialogs are usually available with frame grabbers
				controller->showCaptureDialogDisplay();
				controller->showCaptureDialogFormat();
				controller->showCaptureDialogSource();
				//Get the number of input channels (framegrabbes)
				//The input channel can be changed with controller->setInputChannel
				numInputchannels = controller->getInputChannelsCount();
				if ( numInputchannels > 0 )
					cout << "The device has " << numInputchannels << " input channels" <<endl;
				
				if ( controller->supportCameraControl() )
				{
					cout << "Camera properties:" << endl;
					//We can get and modify the camera properties, for example the Exposure
					printCameraControl( ICaptureDeviceDSController::Exposure, "Exposure" );
					printCameraControl( ICaptureDeviceDSController::Iris, "Iris" );
					printCameraControl( ICaptureDeviceDSController::Focus, "Focus" );
					printCameraControl( ICaptureDeviceDSController::Zoom, "Zoom" );
					printCameraControl( ICaptureDeviceDSController::Pan, "Pan" );
					printCameraControl( ICaptureDeviceDSController::Tilt, "Tilt" );
					printCameraControl( ICaptureDeviceDSController::Roll, "Roll" );
				}
				else
					cout << "This camera doesn't support camera control" << endl;
				
				cout << endl;
				if ( controller->supportImageProperties() )
				{
					cout << "Image properties:" << endl;					
					//We can get and modify the image properties, for example the Brightness
					printImageProperty( ICaptureDeviceDSController::Brightness, "Brightness" );
					printImageProperty( ICaptureDeviceDSController::Contrast, "Contrast" );
					printImageProperty( ICaptureDeviceDSController::Hue, "Hue" );
					printImageProperty( ICaptureDeviceDSController::Saturation, "Saturation" );
					printImageProperty( ICaptureDeviceDSController::Gamma, "Gamma" );
					printImageProperty( ICaptureDeviceDSController::ColorEnable, "ColorEnable" );
					printImageProperty( ICaptureDeviceDSController::WhiteBalance, "WhiteBalance" );
					printImageProperty( ICaptureDeviceDSController::BacklightCompensation, "BacklightCompensation" );
					printImageProperty( ICaptureDeviceDSController::Gain, "Gain" );
				}
				else
					cout << "This camera doesn't support image properties control" << endl;

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
	clear();
	return 0;
}