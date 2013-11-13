#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMYOURINPUTNAMEInput.h"
#include "VMYOURINPUTNAME.h"

// DEFINE THE IDENTIFIERS THE MODULE SUPPORTS
const char *identifiers[1] = {"YOURINPUTNAME_CAPTURE_DEVICE"};
const char **controllersIdentifiers = NULL;

const int numIdentifiers = 1;
const int numControllersIdentifiers=0;

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
extern "C" MODULENAME_API VideoManPrivate::VideoInput *initVideoInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );
VideoManPrivate::VideoInput *initVideoInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format )
{
	VMYOURINPUTNAMEInput *input = new VMYOURINPUTNAMEInput();
	VideoMan::VMInputFormat f;
	if ( input->init("","",&f) ) //you can use any inputIdentification field
		return input;
	delete input;
	return NULL;
}

/*** METHOD FOR KNOWING THE LIST OF DEVICES THE MODULE CAN HANDLE ***/
/*** THIS METHOD COULD NOT BE DECLARED FOR VIDEO FILE INPUT MODULES FOR EXAMPLE ***/
extern "C" MODULENAME_API void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices );
void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices )
{	
}

/*** METHOD FOR RELEASING THE MEMORY USED FOR THE DEVICE LIST ***/
/*** THIS METHOD COULD NOT BE DECLARED FOR VIDEO FILE INPUT MODULES FOR EXAMPLE ***/
extern "C" __declspec(dllexport) void freeAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices );
void freeAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices )
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
extern "C" MODULENAME_API VideoManPrivate::VideoManInputController *createController( const char *identifier  );
VideoManPrivate::VideoManInputController *createController( const char *identifier )
{
	return NULL;
}


/*** METHOD FOR DELETING A INPUT CREATED BY THIS MODULE ***/
extern "C" MODULENAME_API void deleteInput( VideoManPrivate::VideoInput **input );
void deleteInput( VideoManPrivate::VideoInput **input )
{
	delete *input;			
	*input = NULL;
}

/*** METHOD FOR DELETING A CONTROLLER CREATED BY THIS MODULE ***/
extern "C" MODULENAME_API void deleteController( VideoManPrivate::VideoManInputController **controller );
void deleteController( VideoManPrivate::VideoManInputController **controller )
{
	delete *controller;			
	*controller = NULL;
}