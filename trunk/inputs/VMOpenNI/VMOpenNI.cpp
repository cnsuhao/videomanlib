#ifdef WIN32
	#include <windows.h>
#endif

#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include "KinectCamera.h"

#include <string>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"VMOpenNI"};
const int numIdentifiers = 1;
const char *controllersIdentifiers[1] = {"VMOpenNI_CONTROLLER"};	
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
	

	//VMInputFormat *f;


	KinectCamera *camera;
	camera = new KinectCamera( );
	if ( !camera->initCamera(device.friendlyName))//, "C:\\SamplesConfig.xml") )
	{		
		delete camera;
		return NULL;		
	}
	if (format !=NULL) camera->getFormat(*format);
	VideoInput *video = camera;
	return camera;	
}

void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	VMInputIdentification *dl;
	dl= new VMInputIdentification[4];
	
	copyStringToChar( "KINECT_Depth_Camera", &dl[0].friendlyName );
	copyStringToChar( "KINECT_Depth_Camera_Color",&dl[1].friendlyName );
	copyStringToChar( "KINECT_RGB_Camera",&dl[2].friendlyName );
	copyStringToChar( "KINECT_IR_Camera",&dl[3].friendlyName );

	copyStringToChar(identifiers[0], &dl[0].identifier );
	copyStringToChar(identifiers[0],&dl[1].identifier );
	copyStringToChar(identifiers[0],&dl[2].identifier );
	copyStringToChar(identifiers[0],&dl[3].identifier );

	*deviceList = dl;
	numDevices=4;

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
	if ( identifier == "VMOpenNI_CONTROLLER" )
	{		
	
	//	return (VideoManInputController*)controller;
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

