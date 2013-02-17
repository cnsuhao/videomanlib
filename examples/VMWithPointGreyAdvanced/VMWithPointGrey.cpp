#ifdef WIN32
	#include <windows.h>
#endif
#include <GL/freeglut.h>
#include <iostream>
#include <string>
#include <vector>
#include "VideoManControl.h"
#include "controllers/IPointGreyController.h"

using namespace std;
using namespace VideoMan;

/*
This is an example that shows how to use PointGrey cameras 
To use this example, VideoMan must be built with the directive VM_OGLRenderer
You also need to build the VMPointGrey2 input
*/

VideoManControl videoMan;
bool fullScreened;

int inputID; //Index of the video input

//In order to use frame callback you must use PGR_CAMERA2 and POINTGREY_CONTROLLER2
bool useFrameCallback = true;
void frameCallback( char *frame, size_t input, double timeStamp, void *data );
IPointGreyController *controller;

int xROI, yROI, widthROI, heightROI;
int xNewROI, yNewROI;
bool roiSelected = false;

void glutResize(int width, int height)
{
	//Notify to VideoMan the change of the screen size
	videoMan.changeScreenSize( 0, 0, width, height );
}

void glutMouseFunc( int button, int mouseState, int x, int y )
{
	float xf = (float)x;
	float yf = (float)y;
	//Transform from screen coordinates to image coordinates
	int input = videoMan.screenToImageCoords( xf, yf );
	//Draw a new point
	if ( mouseState == GLUT_DOWN && button == GLUT_LEFT_BUTTON )
	{
		//Check if the point is inside the input region
		if ( input == inputID )
		{
			xNewROI = xf;
			yNewROI = yf;
			//roiSelected = false;
		}
	}
	else if ( mouseState == GLUT_UP && button == GLUT_LEFT_BUTTON )
	{		
		int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
		controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos, 0 );
		/*
		//ROI format restrictions
		Hmax = Hunit * n = Hposunit*n3 (n, n3 are integers)
		Vmax = Vunit * m = Vposunit*m3 (m, m3 are integers)
		xROI = Hposunit * n1
		yROI = Vposunit * m1
		widthROI = Hunit * n2
		heightROI = Vunit * m2 (n1, n2, m1, m2 are integers)
		xROI + widthROI <= Hmax - Hunit * 2
		yROI + heightROI <= Vmax - Vunit * 2*/
		xNewROI = Hpos * ( xNewROI / Hpos );
		yNewROI = Vpos * ( yNewROI / Vpos );
		int newWidthROI = xf - xNewROI + 1;
		newWidthROI = Hunit * ( newWidthROI / Hunit );
		int newHeightROI = yf - yNewROI + 1;
		newHeightROI = Vunit * ( newHeightROI / Vunit );
		if ( newWidthROI > 0 && newHeightROI > 0 && 
			xNewROI + newWidthROI <= Hmax - Hunit * 2 &&
			yNewROI + newHeightROI <= Vmax - Vunit * 2 &&
			controller->setImageROI( xNewROI, yNewROI, newWidthROI, newHeightROI, 0 ) )
		{
			xROI = xNewROI;
			yROI = yNewROI;
			widthROI = newWidthROI;
			heightROI = newHeightROI;
			videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
			roiSelected = true;
		}
	}
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
			glutLeaveMainLoop();
			break;
		}
		case 's':
		{
			videoMan.showPropertyPage( inputID );
			break;
		}
		case 'g':
		case 'G':
		{
			bool autoGain = controller->getGainControl();
			controller->setGainControl( !autoGain );
			break;
		}
		case 'r':
		case 'R':
		{
			if ( roiSelected && controller->resetImageROI() )
			{
				videoMan.resetImageROI( inputID );
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
		case GLUT_KEY_UP:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos, 0 );				
				if ( yROI + Vpos + heightROI <= Vmax - Vunit * 2 && controller->moveImageROI( xROI, yROI + Vpos ) )
				{
					yROI += Vpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
				}
			}
			break;
		}
		case GLUT_KEY_DOWN:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos, 0 );				
				if ( yROI - Vpos >= 0 && controller->moveImageROI( xROI, yROI - Vpos ) )
				{
					yROI -= Vpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
				}
			}
			break;
		}
		case GLUT_KEY_LEFT:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos, 0 );				
				if ( xROI - Hpos >= 0 && controller->moveImageROI( xROI - Hpos, yROI ) )
				{
					xROI -= Hpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
				}
			}
			break;
		}
		case GLUT_KEY_RIGHT:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos, 0 );				
				if ( xROI + Hpos + widthROI <= Hmax - Hunit * 2 && controller->moveImageROI( xROI + Hpos, yROI ) )
				{
					xROI += Hpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );					
				}
			}
			break;
		}
    }
}


bool InitializeVideoMan()
{
	inputID = -1;
	VMInputFormat format;
	VMInputIdentification *deviceList;
	int numDevices;

	videoMan.getAvailableDevices( "PGR_CAMERA2", &deviceList, numDevices );//try also PGR_CAMERA
	cout << "Found " << numDevices << " PointGrey cameras:" << endl;
	for ( int d = 0; d < numDevices; ++d )
		cout << "-" << deviceList[d].friendlyName << " with serial " << deviceList[d].uniqueName << endl;
	for ( int d = 0; d < numDevices; ++d )
	{
		format.showDlg = true;	//The user select the format with the Dialog
		inputID = videoMan.addVideoInput( deviceList[d], &format );
		if ( inputID != -1 )
		{
			cout << " Initialized camera " << deviceList[d].friendlyName << " " << deviceList[d].uniqueName << endl;
			videoMan.setVerticalFlip( inputID, true );
			
			//setFrameCallback only woks for PGR_CAMERA2
			if ( useFrameCallback )
				videoMan.setFrameCallback( inputID, frameCallback, NULL );
			
			//Get the controller to access advanced functionality
			controller = (IPointGreyController*)videoMan.createController( inputID, "POINTGREY_CONTROLLER2" );//try POINTGREY_CONTROLLER2
			//The controller must be destroyed with deleteController
			if ( controller )
			{
				controller->printCameraInfo();
				controller->setGainControl( true );
				controller->setShutterTime( 10 );
			}
			break;
		}
		++d;
	}
	videoMan.freeAvailableDevicesList( &deviceList, numDevices );	

	return ( inputID != -1 );
}

void frameCallback( char *frame, size_t input, double timeStamp, void *data )
{
	glutPostRedisplay();
}

void glutIdle()
{
	//Get a new frame
	if ( videoMan.getFrame( inputID ) != NULL )
		glutPostRedisplay();
}

void glutDisplay(void)
{
	glClear( GL_COLOR_BUFFER_BIT );	
	videoMan.updateTexture( inputID );
	videoMan.releaseFrame( inputID );
	videoMan.renderFrame( inputID );
    glutSwapBuffers();
}

void showHelp()
{
	cout << "========" << endl;
	cout << "Controls:" << endl;
	cout << "Esc->Exit" << endl;
	cout << "F1->Fullscreen" << endl;
	cout << "S->Show camera properties" << endl;
	cout << "G->Turn on/off auto gain control" << endl;	
	cout << "LEFT Mouse button: Define region of interest (ROI)" << endl;
	cout << "UP/DOWN->Move the ROI up/down" << endl;
	cout << "LEFT/RIGHT->Move the ROI left/right" << endl;		
	cout << "R->Reset image ROI" << endl;	
	cout << "========" << endl;
}

int main(int argc, char** argv)
{
	cout << endl << "=====================================================" << endl;	
	cout << "This is an example that shows how to use PointGrey cameras and the advanced functionality" << endl;	
	cout << "Works with VMPointGrey (using FlyCapture v1) and VMPointGrey2 (using FlyCapture v2)" << endl;	
	glutInitDisplayMode( GLUT_DOUBLE | GLUT_DEPTH | GLUT_RGBA | GLUT_MULTISAMPLE );
	glutInitWindowPosition( 0, 0 );
	glutInitWindowSize( 640, 480 );
    glutInit( &argc, argv );
    glutCreateWindow("VideoMan with PointGrey");
	glutReshapeFunc(glutResize );  
	//glutIdleFunc(glutDisplay);
    glutDisplayFunc(glutDisplay);	
    glutKeyboardFunc(glutKeyboard);
	glutSpecialFunc(glutSpecialKeyboard);
	glutMouseFunc( glutMouseFunc );
	glutSetOption( GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_CONTINUE_EXECUTION); 

	if ( !InitializeVideoMan() )
		return -1; 

	if ( !useFrameCallback )
		glutIdleFunc(glutIdle);	

	fullScreened = false;	
	showHelp();    
	glutMainLoop();
	clear();
	return 0;
}
