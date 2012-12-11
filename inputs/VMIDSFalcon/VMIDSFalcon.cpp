#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
	#include <windows.h>
#endif
#include <iostream>
#include <sstream>

#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include "IDSFalconFrameGrabber.h"
#include "falconFrameGrabberController.h"

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"IDS_FALCON_FRAME_GRABBER"};
const int numIdentifiers = 1;
const char *controllersIdentifiers[1] = {"IDS_FALCON_FRAME_GRABBER_CONTROLLER"};	
const int numControllersIdentifiers = 1;

#ifdef linux
	void __attribute__ ((constructor)) my_init(void);
	void __attribute__ ((destructor)) my_fini(void); 

void my_init()
{
}

void my_fini()
{
}
#endif

#ifdef WIN32
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_THREAD_ATTACH:        
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:		
		break;
	}

    return TRUE;
}
#endif

//extern "C" __declspec(dllexport) VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format );

VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	IDSFalconFrameGrabber *grabber = new IDSFalconFrameGrabber();	
	int cameraId = 0;
	if ( device.uniqueName )
	{
		std::istringstream ss ( device.uniqueName );
		ss >> cameraId;
	}
	if ( grabber->initBoard( cameraId, format ) )
		return grabber;
    
 	return NULL;
}

void getAvailableDevices( VMInputIdentification **_deviceList, int &numDevices )
{
	IDSFalconFrameGrabber::getAvailableDevices( _deviceList, numDevices );
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
	delete *deviceList;
	*deviceList = NULL;
	numDevices = 0;
}

const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

//extern "C" __declspec(dllexport) char *checkVersion( );
char* checkVersion( )
{
	#ifdef _DEBUG
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

VideoManInputController *createController( const char *identifier )
{
	if ( identifier == NULL )
		return NULL;
	std::string id = identifier;
	if ( id == controllersIdentifiers[0] )
	{
		FalconFrameGrabberController *controller = new FalconFrameGrabberController( "controllersIdentifiers[0]" );		 
		return (VideoManInputController*)controller;
	}	
	return NULL;
}

void deleteController( VideoManInputController **controller )
{
	delete (*controller);
	(*controller) = NULL;
}
