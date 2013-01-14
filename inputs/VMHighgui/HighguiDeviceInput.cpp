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
	cvReleaseCapture( &capture );
}

bool HighguiDeviceInput::initInput( const VMInputIdentification &device, VMInputFormat *aformat )
{	
	int id = 0;
	if ( device.uniqueName )
	{
		istringstream ss( device.uniqueName );
		ss >> id;
	}
	capture = cvCreateCameraCapture( id );
	if( capture )
	{
		if ( aformat )
		{
			cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH, aformat->width );
			cvSetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT, aformat->height ); 
			cvSetCaptureProperty( capture, CV_CAP_PROP_FPS, aformat->fps  );		
		}
		format.width = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH );
		format.height = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT );
		format.fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );		

		IplImage *image = cvQueryFrame( capture );
		if ( !image ) 
			return false;
		format.nChannels = image->nChannels;
		format.depth = image->depth;
		if ( image->nChannels == 3 )
		{
			string channelSeq = image->channelSeq;
			if ( channelSeq == "BGR" )
				format.setPixelFormat( BGR24, BGR24 );
			else if ( channelSeq == "RGB " )
				format.setPixelFormat( RGB24, RGB24 );
		}
		else if ( image->nChannels == 1 )
		{
			if ( image->depth == 8 )
				format.setPixelFormat( GREY8, GREY8 );
			else if ( image->depth == 16 )
				format.setPixelFormat( GREY16, GREY16 );
		}
		if ( aformat ) 
			*aformat= format;
		pixelBuffer = image->imageData;
		
		const string identifier = "HIGHGUI_CAPTURE_DEVICE";
		identification.identifier = new char[identifier.length() + 1];
		strcpy( identification.identifier, identifier.c_str() );

		return true;
	}
	return false;
}

void HighguiDeviceInput::releaseFrame( )
{
	cvGrabFrame( capture );
	pixelBuffer = NULL;
}

char *HighguiDeviceInput::getFrame( bool wait)
{
	IplImage *image = cvRetrieveFrame( capture );//cvQueryFrame( capture );
	pixelBuffer = image->imageData;
	return pixelBuffer;
}
