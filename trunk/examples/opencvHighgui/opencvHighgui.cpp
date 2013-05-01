#ifdef WIN32
	#include <windows.h>
#endif
#include <iostream>
#ifdef linux
	#include <stdio.h>
#endif

#include "cxcore.h"
#include "highgui.h"

#include "VideoManControl.h"

using namespace std;
using namespace VideoMan;

/*
This is a simple example using OpenCV Highgui for the image visualization without OpenGL Renderer.
*/

VideoManControl *videoMan;
int videoInputID; //Index of the video input
char *videoFile = 0;

IplImage *inputHeader;

bool InitializeVideoMan()
{
	VMInputFormat format;
	VMInputIdentification device;
	if ( videoFile )
	{
		//Initialize one input from a video file
		device.fileName = videoFile;
		if ( videoMan->supportedIdentifier( "DSHOW_VIDEO_FILE" ) )
			device.identifier = "DSHOW_VIDEO_FILE"; //using directshow
		else if ( videoMan->supportedIdentifier( "HIGHGUI_VIDEO_FILE" ) )
			device.identifier = "HIGHGUI_VIDEO_FILE"; //using highugui
		//play in real-time
		format.clock = true;
		format.renderAudio = true;
		//Initialize the video file is the path
		if ( ( videoInputID = videoMan->addVideoInput( device, &format ) ) != -1 )
		{
			if ( device.fileName )
				printf("Loaded video file: %s\n", device.fileName );
			printf("resolution: %d %d\n", format.width, format.height );

			//get the length of the video
			double videoLength = videoMan->getLengthSeconds( videoInputID );
			printf("duration: %f seconds\n\n", videoLength );

			videoMan->playVideo( videoInputID );
		}
	}
	else
	{
		//Initialize one input from a camera
		VMInputIdentification *list;
		int numDevices;
		videoMan->getAvailableDevices( &list, numDevices ); //list all the available devices
		if ( numDevices == 0 )
		{
			printf("There is no available camera\n");
			videoMan->freeAvailableDevicesList(  &list, numDevices );
			return false;
		}
		int d= 0;
		videoInputID = -1;
		while ( d < numDevices && videoInputID == -1 )
		{
			device = list[d];
			//Show dialog to select the format
			format.showDlg = true;
			if ( ( videoInputID = videoMan->addVideoInput( device, &format ) ) != -1 )
			{
				//videoMan->showPropertyPage( videoInputID );
				videoMan->getFormat( videoInputID, format );
				if ( device.friendlyName )
					printf("Initilized camera: %s\n", device.friendlyName );
				printf("resolution: %d %d\n", format.width, format.height );
				printf("FPS: %f\n\n", format.fps );
			}
			++d;
		}
		videoMan->freeAvailableDevicesList(  &list, numDevices );
	}
	if ( videoInputID != -1 )
	{
		inputHeader = cvCreateImageHeader( cvSize( format.width, format.height), format.depth, format.nChannels );
		inputHeader->origin = 1;
		return true;
	}
	return false;
}

void showHelp()
{
	printf("========\n");
	printf("keys:\n");
	printf("Esc->Exit\n");
	printf("========\n");
}

int main(int argc, char** argv)
{
	cout << "This is a simple example using OpenCV Highgui for the window creation and image visualization" << endl;
	cout << "Usage: opencvHighgui.exe filePath(string)" << endl;
	cout << "Example: opencvHighgui.exe c:\\video.avi" << endl;
	cout << "If you don't specify a filepath, a camera will be initialized" << endl;
	cout << "=====================================================" << endl;

	//The OpenGL renderer is not needed
	videoMan = new VideoManControl( VideoManControl::NO_RENDERER );

	if ( argc > 1 )
		videoFile = argv[1];

	if ( !InitializeVideoMan() )
	{
		showHelp();
		delete videoMan;
		cout << "Error intializing VideoMan" << endl;
		cout << "Pres Enter to exit" << endl;
		getchar();
		return -1;
	}

	showHelp();
	cvNamedWindow( "VideoMan example with Highgui" );
	char code = -1;
	for(;;)
	{
		char *img = videoMan->getFrame( videoInputID );
		if( img )
		{
			inputHeader->imageData = img;
			cvShowImage( "VideoMan example with Highgui", inputHeader );
			videoMan->releaseFrame( videoInputID );
		}
		code = (char) cvWaitKey( 10 );
        if( code == 27 || code == 'q' || code == 'Q' )
			break;
	}

	cvDestroyWindow( "VideoMan example with Highgui" );
	cvReleaseImageHeader( &inputHeader );
	delete videoMan;

	return 0;
}
