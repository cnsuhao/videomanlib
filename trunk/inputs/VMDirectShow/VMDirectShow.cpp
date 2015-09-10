#include "stdafx.h"
#include <iostream>
#include <string>
#include <vector>

#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VideoFileDShow.h"
#include "CaptureDeviceDShow.h"
#include "CaptureDeviceDSController.h"
#include "VMmoduleBase.h"

const char *identifiers[2] = {"DSHOW_CAPTURE_DEVICE","DSHOW_VIDEO_FILE"};
const int numIdentifiers = 2;

#ifdef IDS_UEYE
	#include "DSuEyeCameraController.h"
#endif
#ifdef DS_POINTGREY
	#include "DSPointGreyController.h"
#endif

const int numControllersIdentifiers = 3;
const char *controllersIdentifiers[] = {"DSHOW_CAPTURE_DEVICE_CONTROLLER","DSHOW_uEYE_CAMERA_CONTROLLER","DSHOW_POINTGREY_CONTROLLER"};	

using namespace VideoMan;
using namespace VideoManPrivate;

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{	
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			CoInitialize(NULL);	
			break;
		}
	case DLL_THREAD_ATTACH:        
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			CoUninitialize();			
			break;
		}
	}

    return TRUE;
}

void freeChar( char **src )
{
	if ( *src )
	{
		delete [] *src;
		*src = NULL;
	}
}

VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	std::string identifier = device.identifier;
	if ( identifier == "DSHOW_VIDEO_FILE" ) 
	{
		VideoFileDShow *videoFile;
		videoFile = new VideoFileDShow();
		if (!videoFile->loadVideoFile( device.fileName, format ))
		{
			delete videoFile;
			return NULL;
		}
		return videoFile;
	}
	else if ( identifier == "DSHOW_CAPTURE_DEVICE" )
	{
		CaptureDeviceDShow *capture;
		capture = new CaptureDeviceDShow();
		if (!capture->initCamera( device.friendlyName, device.uniqueName, format ))
		{
			delete capture;
			return NULL;
		}
		return capture;
	}
 	return NULL;
}

void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{	
	CaptureDeviceDShow::getAvailableDevices(deviceList, numDevices );	
}

void freeAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	if ( *deviceList == NULL )
		return;
	for ( int d = 0; d < numDevices; ++d )
	{
		freeChar( &(*deviceList)[d].uniqueName );
		freeChar( &(*deviceList)[d].fileName );
		freeChar( &(*deviceList)[d].friendlyName );
		freeChar( &(*deviceList)[d].identifier );
	}	
	delete [] *deviceList;
	*deviceList = NULL;
	numDevices = 0;
}

const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

const char **getControllerIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers = numControllersIdentifiers;
	return controllersIdentifiers;
}

VideoManInputController *createController( const char *identifier )
{
	if ( identifier == NULL )
		return NULL;
	std::string id = identifier;
	if ( id == "DSHOW_CAPTURE_DEVICE_CONTROLLER" )
	{
		CaptureDeviceDSController *controller = new CaptureDeviceDSController( "DSHOW_CAPTURE_DEVICE_CONTROLLER" );		 
		return (VideoManInputController*)controller;
	}
	#ifdef IDS_UEYE
	else if ( id == "DSHOW_uEYE_CAMERA_CONTROLLER" )
	{
		DSuEyeCameraController *controller = new DSuEyeCameraController( "DSHOW_uEYE_CAMERA_CONTROLLER" );
		return (VideoManInputController*)controller;
	}	
	#endif
	#ifdef DS_POINTGREY
	else if ( id == "DSHOW_POINTGREY_CONTROLLER" )
	{
		DSPointGreyController *controller = new DSPointGreyController( "DSHOW_POINTGREY_CONTROLLER" );
		return (VideoManInputController*)controller;
	}	
	#endif
	return NULL;
}


char* checkVersion( )
{
	#ifndef NDEBUG
		return "DEBUG" ;			
	#endif
	#ifdef NDEBUG
		return "RELEASE";			
	#endif
}

void deleteInput( VideoInput **input )
{
	delete (*input);
	(*input) = NULL;
}

void deleteController( VideoManInputController **controller )
{
	delete (*controller);
	(*controller) = NULL;
}

