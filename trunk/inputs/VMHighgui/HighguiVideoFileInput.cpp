#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "HighguiVideoFileInput.h"
#include <iostream>
#include <string>

using namespace std;
using namespace VideoMan;

HighguiVideoFileInput::HighguiVideoFileInput(void)
{
	paused = false;
	step = false;
}

HighguiVideoFileInput::~HighguiVideoFileInput(void)
{
	cvReleaseCapture( &capture );
}

bool HighguiVideoFileInput::initInput( const VMInputIdentification &device, VMInputFormat *aformat )
{	
	if ( device.fileName == NULL )
		return false;
	string fileName = device.fileName;
	capture = cvCaptureFromFile( fileName.c_str() );
	if( capture )
	{
		format.width = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_WIDTH );
		format.height = (int)cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_HEIGHT );
		format.fps = cvGetCaptureProperty( capture, CV_CAP_PROP_FPS );		
		videoLengthFrames = static_cast<int>( cvGetCaptureProperty( capture, CV_CAP_PROP_FRAME_COUNT ) );
		videoLengthSeconds = 0;
		if ( format.fps != 0 )
			videoLengthSeconds = static_cast<double>( videoLengthFrames ) / format.fps;	
		IplImage *image = cvQueryFrame( capture );
		format.nChannels = image->nChannels;
		format.depth = image->depth;
		cvSetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES, 0 );
		if ( image->nChannels == 3 )
		{
			string channelSeq = image->channelSeq;
			if ( channelSeq == "BGR" )
				format.setPixelFormat( VM_BGR24, VM_BGR24 );
			else if ( channelSeq == "RGB " )
				format.setPixelFormat( VM_RGB24, VM_RGB24 );
		}
		else if ( image->nChannels == 1 )
		{
			if ( image->depth == 8 ) 
				format.setPixelFormat( VM_GREY8, VM_GREY8 );
			else if ( image->depth == 16 ) 
				format.setPixelFormat( VM_GREY16, VM_GREY16 );

		}
		//pixelBuffer = image->imageData;
		//format.SetFormat( image->width, image->height, 30, VM_RGB24, VM_RGB24 );	
		if ( aformat )
			*aformat= format;
		cvGrabFrame( capture );
		
		const string identifier = "HIGHGUI_VIDEO_FILE";
		identification.identifier = new char[identifier.length() + 1];
		strcpy( identification.identifier, identifier.c_str() );
	
		identification.fileName = new char[fileName.length() + 1];
		strcpy( identification.fileName, fileName.c_str() );

		return true;
	}
	return false;
}

void HighguiVideoFileInput::releaseFrame( )
{
	cvGrabFrame( capture );
	pixelBuffer = NULL;
}

char *HighguiVideoFileInput::getFrame( bool wait)
{
	if ( !paused || step )
	{
		IplImage *image = cvRetrieveFrame( capture );//cvQueryFrame( capture );
		if (image) pixelBuffer = image->imageData;
		step = false;
	}
	return pixelBuffer;
}

double HighguiVideoFileInput::getLengthSeconds()
{
	return videoLengthSeconds;
}
	
int HighguiVideoFileInput::getLengthFrames()
{
	return videoLengthFrames;
}

double HighguiVideoFileInput::getPositionSeconds()
{
	return cvGetCaptureProperty( capture, CV_CAP_PROP_POS_MSEC ) / 1000.0;	
}
	
int HighguiVideoFileInput::getPositionFrames()
{
	return static_cast<int>( cvGetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES ) );
}

void HighguiVideoFileInput::goToFrame( int frame )
{
	cvSetCaptureProperty( capture, CV_CAP_PROP_POS_FRAMES, frame );
	step = true;
}
	
void HighguiVideoFileInput::goToMilisecond( double milisecond )
{
	cvSetCaptureProperty( capture, CV_CAP_PROP_POS_MSEC, milisecond );
	step = true;
}

void HighguiVideoFileInput::play()
{
	paused = false;
}

void HighguiVideoFileInput::pause()
{
	paused = true;
}