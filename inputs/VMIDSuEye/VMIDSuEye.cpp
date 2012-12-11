#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include "IDSuEye.h"
#include "uEyeCameraController.h"
#include <vector>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"IDS_uEye_CAMERA"};
const int numIdentifiers = 1;
const char *controllersIdentifiers[1] = {"uEye_CAMERA_CONTROLLER"};	
const int numControllersIdentifiers = 1;

#ifdef WIN32
#define VMIDSuEye_API  __declspec(dllexport)
#elif defined linux
#define VMIDSuEye_API 
#endif

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
	availableDevices = NULL;
	numAvailableDevices = 0;
}
void my_fini()
{
	freeAvailableDevicesList( &availableDevices, numAvailableDevices );
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
#endif

//extern "C" VMIDSuEye_API VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format );
VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	IDSuEye *cam = new IDSuEye();
	if ( cam->initInput( device, format ) )
	{
		return cam;
	}
	else
	{
		delete cam;
		return NULL;
	}
}

//extern "C" VMIDSuEye_API void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices );
void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	IDSuEye::getAvailableDevices( deviceList, numDevices );	
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

//extern "C" VMIDSuEye_API const char **getIdentifiers( int &numIdentifiers );
const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

//extern "C" VMIDSuEye_API char* checkVersion( );
char* checkVersion( )
{
	#ifdef _DEBUG
		return "DEBUG" ;			
	#endif
	#ifdef NDEBUG
		return "RELEASE";			
	#endif
}

//extern "C" VMIDSuEye_API void deleteInput( VideoInput **input );
void deleteInput( VideoInput **input )
{
	delete (*input);
	(*input) = NULL;
}

//extern "C" VMIDSuEye_API const char **getControllerIdentifiers( int &numIdentifiers );
const char **getControllerIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers = numControllersIdentifiers;
	return controllersIdentifiers;
}

//extern "C" VMIDSuEye_API VideoManInputController *createController( const char *identifier  );
VideoManInputController *createController( const char *identifier )
{
	if ( identifier == NULL )
		return NULL;
	std::string id = identifier;
	if ( id == "uEye_CAMERA_CONTROLLER" )
	{
		uEyeCameraController *controller = new uEyeCameraController( "uEye_CAMERA_CONTROLLER" );		 
		return (VideoManInputController*)controller;
	}	
	return NULL;
}

//extern "C" VMIDSuEye_API void deleteController( VideoManInputController **controller );
void deleteController( VideoManInputController **controller )
{
	delete (*controller);
	(*controller) = NULL;
}
