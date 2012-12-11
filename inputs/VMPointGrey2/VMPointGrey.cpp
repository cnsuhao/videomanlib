#ifdef WIN32
	#include <windows.h>
#endif

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

const char *identifiers[1] = {"PGR_CAMERA2"};
const int numIdentifiers = 1;
const char *controllersIdentifiers[1] = {"POINTGREY_CONTROLLER2"};	
const int numControllersIdentifiers = 1;

/*
#ifdef WIN32
#define VMPTGREY_API  __declspec(dllexport)
#elif defined linux
#define VMPTGREY_API 
#endif
*/
void freeChar( char **src )
{
	if ( *src )
	{
		delete *src;
		*src = NULL;
	}
}


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
#include <windows.h>
/*** MODULE INITIALIZATION AND FINALIZATION  ***/
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

void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	PGRCamera::getAvailableDevices( deviceList, numDevices );
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
	if ( identifier == "POINTGREY_CONTROLLER2" )
	{		
		PointGreyController *controller = new PointGreyController( "POINTGREY_CONTROLLER2" );
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

