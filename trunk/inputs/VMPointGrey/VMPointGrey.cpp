#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
// Windows Header Files:
#include <windows.h>

#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include "PGRCamera.h"
#include "PointGreyController.h"

#include <string>
#include <sstream>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"PGR_CAMERA"};
const int numIdentifiers = 1;
const char *controllersIdentifiers[1] = {"POINTGREY_CONTROLLER"};	
const int numControllersIdentifiers = 1;


void freeChar( char **src )
{
	if ( *src )
	{
		delete *src;
		*src = NULL;
	}
}

#define VMPTGREY_API  __declspec(dllexport)

BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			break;
		}
	case DLL_THREAD_ATTACH:        
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:						
		{
			break;
		}
	}
    return TRUE;
}

//extern "C" __declspec(dllexport) VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format );

VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	PGRCamera *camera;
	camera = new PGRCamera( );
	unsigned long serialNumber = 0;
	if ( device.uniqueName )
	{
		istringstream ss( device.uniqueName );
		ss >> serialNumber;
	}
	if ( !camera->initCamera( serialNumber, format ) )
	{		
		delete camera;
		return NULL;		
	}
	VideoInput *video = camera;
	return camera;	
}

//extern "C" VMPTGREY_API void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices );
void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	PGRCamera::getAvailableDevices( deviceList, numDevices );
}

//extern "C" __declspec(dllexport) void freeAvailableDevices( VMInputIdentification **deviceList, int &numDevices );
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

//extern "C" VMPTGREY_API const char **getIdentifiers( int &numIdentifiers );
const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

//extern "C" __declspec(dllexport) char *checkVersion( );
char *checkVersion( )
{
	#ifdef _DEBUG
		return "DEBUG" ;			 
	#endif
	#ifdef NDEBUG
		return "RELEASE";			
	#endif
}

const char **getControllerIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers = numControllersIdentifiers;
	return controllersIdentifiers;
}

VideoManInputController *createController( const char *_identifier  )
{
	if ( _identifier == NULL )
		return NULL;
	string identifier = _identifier;
	if ( identifier == "POINTGREY_CONTROLLER" )
	{		
		PointGreyController *controller = new PointGreyController( "POINTGREY_CONTROLLER" );
		return (VideoManInputController*)controller;
	}
	return NULL;
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