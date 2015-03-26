#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "HighguiDeviceInput.h"
#include <iostream>
#include <string>

using namespace std;
using namespace VideoMan;

HighguiDeviceInput::HighguiDeviceInput(void)
{

}

HighguiDeviceInput::~HighguiDeviceInput(void)
{
	capture.release();	
}

bool HighguiDeviceInput::initInput( const VMInputIdentification &device, VMInputFormat *aformat )
{	
	int id = 0;
	if ( device.uniqueName )
	{
		istringstream ss( device.uniqueName );
		ss >> id;
	}
	capture.open( id );
	if( capture.isOpened() )
	{
		if ( aformat )
		{
			//set prefered format
			capture.set( CV_CAP_PROP_FRAME_WIDTH, aformat->width );
			capture.set( CV_CAP_PROP_FRAME_HEIGHT, aformat->height ); 
			capture.set( CV_CAP_PROP_FPS, aformat->fps  );		
		}

		format.width = (int)capture.get( CV_CAP_PROP_FRAME_WIDTH );
		format.height = (int)capture.get( CV_CAP_PROP_FRAME_HEIGHT );
		format.fps = capture.get( CV_CAP_PROP_FPS );		

		cv::Mat image;
		if ( !capture.grab() )
			return false;		
		if ( !capture.retrieve( image ) )
			return false;		
		format.width = image.cols;
		format.height = image.rows;
		format.nChannels = image.channels();
		format.depth = image.depth();
		if ( image.channels() == 3 )
		{		
			format.setPixelFormat( VM_BGR24, VM_BGR24 );		
		}
		else if ( image.channels() == 1 )
		{
			if ( image.depth() == 8 )
				format.setPixelFormat( VM_GREY8, VM_GREY8 );
			else if ( image.depth() == 16 )
				format.setPixelFormat( VM_GREY16, VM_GREY16 );
		}
		if ( aformat ) 
			*aformat= format;
		pixelBuffer = (char*)image.data;
		
		const string identifier = "HIGHGUI_CAPTURE_DEVICE";
		identification.identifier = new char[identifier.length() + 1];
		strcpy( identification.identifier, identifier.c_str() );

		std::stringstream ss;
		ss << "Device " << id;

		identification.friendlyName = new char[ss.str().length() + 1];
		strcpy(identification.friendlyName, ss.str().c_str());

		ss.clear();
		ss << id;
		identification.uniqueName = new char[ss.str().length() + 1];
		strcpy(identification.uniqueName, ss.str().c_str());

		return true;
	}
	return false;
}

void HighguiDeviceInput::releaseFrame( ) 
{
	pixelBuffer = NULL;
}

char *HighguiDeviceInput::getFrame( bool wait)
{
	if ( capture.grab() )
	{
		if ( capture.retrieve( image ) )
			pixelBuffer = (char*)image.data;
	}
	return pixelBuffer;
}
