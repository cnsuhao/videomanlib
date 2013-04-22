#ifdef WIN32
	#include <windows.h>
#endif
#include <iostream>
#include <string>
#include <vector>
#include "VideoManControl.h"
#include "controllers/IPointGreyController.h"
#include <SFML/Graphics.hpp>

using namespace std;
using namespace VideoMan;

/*
This is an example that shows how to use PointGrey cameras 
To use this example, VideoMan must be built with the directive VM_OGLRenderer
You also need to build the VMPointGrey2 input
*/

VideoManControl videoMan;
int inputID; //Index of the video input
IPointGreyController *controller;
int xROI, yROI, widthROI, heightROI;
int xNewROI, yNewROI;
bool roiSelected = false;
bool selectingROI = false;

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

void keyboardControl( sf::Window &App, sf::Clock &clock, const sf::Event &Event )
{
	switch( Event.key.code )
	{
		case sf::Keyboard::Escape:
			App.close();
			break;					
		case sf::Keyboard::Left:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos );				
				if ( xROI - Hpos >= 0 && controller->moveImageROI( xROI - Hpos, yROI ) )
				{
					xROI -= Hpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
				}
			}
			break;
		}
		case sf::Keyboard::Right:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos );				
				if ( xROI + Hpos + widthROI <= Hmax - Hunit * 2 && controller->moveImageROI( xROI + Hpos, yROI ) )
				{
					xROI += Hpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );					
				}
			}
			break;
		}
		case sf::Keyboard::Up:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos );				
				if ( yROI + Vpos + heightROI <= Vmax - Vunit * 2 && controller->moveImageROI( xROI, yROI + Vpos ) )
				{
					yROI += Vpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
				}
			}
			break;
		}
		case sf::Keyboard::Down:
		{
			if ( roiSelected )
			{
				int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
				controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos );				
				if ( yROI - Vpos >= 0 && controller->moveImageROI( xROI, yROI - Vpos ) )
				{
					yROI -= Vpos;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
				}
			}
			break;
		}
		case sf::Keyboard::S:
		{
			videoMan.showPropertyPage( inputID );
			break;
		}
		case sf::Keyboard::G:
		{
			bool autoGain = controller->getGainControl();
			controller->setGainControl( !autoGain );
			break;
		}
		case sf::Keyboard::R:
		{
			if ( controller->resetImageROI() )
			{
				//give time to the camera to switch the image ROI
				videoMan.resetImageROI( inputID );							
				clock.restart();
			}		
			break;
		}
	}
}

void mouseControl( VideoManControl &videoMan, sf::Clock &clock, const sf::Event &Event )
{
	float xf = (float)Event.mouseButton.x;
	float yf = (float)Event.mouseButton.y;
	//Transform from screen coordinates to image coordinates
	int input = videoMan.screenToImageCoords( xf, yf );
	//Check if the point is inside the input region
	if ( input == inputID )
	{
		if ( Event.type == sf::Event::MouseButtonPressed )
		{
			xNewROI = (int)xf;
			yNewROI = (int)yf;
			selectingROI = true;
		}			
		else if ( Event.type == sf::Event::MouseButtonReleased )
		{	
			if ( !selectingROI || clock.getElapsedTime().asSeconds() < 0.5 )
				return;
			selectingROI = false;
			int Hmax, Vmax, Hunit, Vunit, Hpos, Vpos;
			if ( controller->getROIUnits( Hmax, Vmax, Hunit, Vunit, Hpos, Vpos ) )
			{
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
				int newWidthROI = static_cast<int>( xf - xNewROI + 1 );
				newWidthROI = Hunit * ( newWidthROI / Hunit );
				int newHeightROI = static_cast<int>( yf - yNewROI + 1 );
				newHeightROI = Vunit * ( newHeightROI / Vunit );
				if ( newWidthROI > 0 && newHeightROI > 0 && 
					xNewROI + newWidthROI <= Hmax - Hunit * 2 &&
					yNewROI + newHeightROI <= Vmax - Vunit * 2 &&
					controller->setImageROI( xNewROI, yNewROI, newWidthROI, newHeightROI ) )
				{
					//give time to the camera to switch the image ROI			
					xROI = xNewROI;
					yROI = yNewROI;
					widthROI = newWidthROI;
					heightROI = newHeightROI;
					videoMan.setImageROI( inputID, xROI, yROI, widthROI, heightROI );
					roiSelected = true;
					clock.restart();
				}
			}
		}
	}
}

int main(int argc, char** argv)
{
	sf::RenderWindow app(sf::VideoMode(800, 600, 24), "VideoMan with PointGrey");
	
	if ( !InitializeVideoMan() )
		return EXIT_FAILURE; 
	videoMan.changeScreenSize( 0, 0, app.getSize().x, app.getSize().y );

	cout << endl << "=====================================================" << endl;	
	cout << "This is an example that shows how to use PointGrey cameras and the advanced functionality" << endl;	
	cout << "Works with VMPointGrey (using FlyCapture v1) and VMPointGrey2 (using FlyCapture v2)" << endl;	
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

	sf::Clock clock;
	while ( app.isOpen() )
	{
		sf::Event newEvent;
		while ( app.pollEvent( newEvent ) )
		{
			if ( newEvent.type == sf::Event::Closed )
				app.close();        
			else if ( ( newEvent.type == sf::Event::MouseButtonPressed || newEvent.type == sf::Event::MouseButtonReleased ) && newEvent.mouseButton.button == sf::Mouse::Left )
				mouseControl( videoMan, clock, newEvent );
			else if ( newEvent.type == sf::Event::KeyPressed )
				keyboardControl( app, clock, newEvent );				
			else if ( newEvent.type == sf::Event::Resized)
				videoMan.changeScreenSize( 0, 0, newEvent.size.width, newEvent.size.height );
		}
		
		app.clear();
		videoMan.getFrame( inputID );
		if ( clock.getElapsedTime().asSeconds() > 0.2 )
			videoMan.updateTexture( inputID );
		videoMan.releaseFrame( inputID );
		videoMan.renderFrame( inputID );
		app.display();
	}

	videoMan.deleteController( (VideoManPrivate::VideoManInputController**)&controller );

	return EXIT_SUCCESS;
}