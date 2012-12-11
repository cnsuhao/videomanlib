#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "YourInputClass.h"
#include "VMmoduleBase.h"

// DEFINE THE IDENTIFIERS THE MODULE SUPPORTS
const char *identifiers[1] = {"MYLIB_CAPTURE_DEVICE","MYLIB_VIDEO_FILE"};
const int numIdentifiers = 2;
// DEFINE THE CONTROLLERS IDENTIFIERS THE MODULE SUPPORTS
const int numControllersIdentifiers = 1;
const char *controllersIdentifiers[1] = {"MYLIB_CAPTURE_DEVICE_CONTROLLER"};	

#ifdef WIN32
#define MODULENAME_API  __declspec(dllexport)
#endif
#ifdef linux
#define MODULENAME_API 
#endif

#ifdef linux
/*** MODULE INITIALIZATION AND FINALIZATION FOR LINUX ***/

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
/*** MODULE INITIALIZATION AND FINALIZATION WINDOWS  ***/
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

/*** INPUTS CREATION METHOD ***/
extern "C" MODULENAME_API VideoInput *initVideoInput( const inputIdentification &device, VideoManInputFormat *format );
VideoInput *initVideoInput( const inputIdentification &device, VideoManInputFormat *format )
{
	YourInputClass *input = new YourInputClass();
	if ( input->yourInitFunction( device.fileName, format ) ) //you can use any inputIdentification field
		return input;
	delete input;
	return NULL;
}

/*** METHOD FOR KNOWING THE LIST OF DEVICES THE MODULE CAN HANDLE ***/
/*** THIS METHOD COULD NOT BE DECLARED FOR VIDEO FILE INPUT MODULES FOR EXAMPLE ***/
extern "C" MODULENAME_API void getAvailableDevices( inputIdentification **deviceList, int &numDevices );
void getAvailableDevices( inputIdentification **deviceList, int &numDevices )
{	
}

/*** METHOD FOR RELEASING THE MEMORY USED FOR THE DEVICE LIST ***/
/*** THIS METHOD COULD NOT BE DECLARED FOR VIDEO FILE INPUT MODULES FOR EXAMPLE ***/
extern "C" __declspec(dllexport) void freeAvailableDevices( inputIdentification **deviceList, int &numDevices );
void freeAvailableDevices( inputIdentification **deviceList, int &numDevices )
{
}

/*** METHOD FOR KNOWING THE LIST OF IDENTIFIERS THE MODULE CAN HANDLE ***/
extern "C" MODULENAME_API const char **getIdentifiers( int &numIdentifiers );
const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

/*** METHOD FOR KNOWING THE LIST OF CONTROLLERS IDENTIFIERS ***/
extern "C" MODULENAME_API const char **getControllerIdentifiers( int &numIdentifiers );
const char **getControllerIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers = numControllersIdentifiers;
	return controllersIdentifiers;
}

/*** FIXED METHOD FOR CHECKING THE VERSION ***/
extern "C" MODULENAME_API char* checkVersion( );
char* checkVersion( )
{
	#ifdef _DEBUG
		return "DEBUG" ;			
	#endif
	#ifdef NDEBUG
		return "RELEASE";			
	#endif
}

/*** METHOD FOR CREATING A CONTROLLER ***/
extern "C" MODULENAME_API VideoManInputController *createController( const char *identifier  );
VideoManInputController *createController( const char *identifier )
{
}

/*** METHOD FOR DELETING A INPUT CREATED BY THIS MODULE ***/
extern "C" MODULENAME_API void deleteInput( VideoInput **input );
void deleteInput( VideoInput **input )
{
	delete *input;			
	*input = NULL;
}

/*** METHOD FOR DELETING A CONTROLLER CREATED BY THIS MODULE ***/
extern "C" MODULENAME_API void deleteController( VideoManInputController **controller );
void deleteController( VideoManInputController **controller )
{
	delete *controller;			
	*controller = NULL;
}