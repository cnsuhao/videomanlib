#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated
#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "HighguiDeviceInput.h"
#include "HighguiVideoFileInput.h"
#include "VMmoduleBase.h"
#include <string>
#include <vector>
#include "VMmoduleBase.h"

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[2] = {"HIGHGUI_CAPTURE_DEVICE","HIGHGUI_VIDEO_FILE"};
const int numIdentifiers = 2;

/*
#ifdef WIN32
#define VMHIGHGUI_API  __declspec(dllexport)
#elif defined linux
#define VMHIGHGUI_API 
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
	//availableDevices = NULL;
	//numAvailableDevices = 0;
}
void my_fini()
{
	//freeAvailableDevicesList( &availableDevices, numAvailableDevices );
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

//extern "C" VMHIGHGUI_API VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format );
VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	std::string identifier = device.identifier;
	if ( identifier == "HIGHGUI_CAPTURE_DEVICE" )
	{
		HighguiDeviceInput *input = new HighguiDeviceInput();
		if ( input->initInput( device, format ) )
		{
			return input;
		}
		delete input;
	}
	else if ( identifier == "HIGHGUI_VIDEO_FILE" )
	{
		HighguiVideoFileInput *input = new HighguiVideoFileInput();
		if ( input->initInput( device, format ) )
		{			
			return input;
		}
		delete input;
	}	
	return NULL;
}

//extern "C" VMHIGHGUI_API void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices );
void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	/*(*deviceList) = new VMInputIdentification[1];
	string identifier = "HIGHGUI_CAPTURE_DEVICE";
	(*deviceList)[0].identifier = new char[identifier.length() + 1];
	strcpy( deviceList[0]->identifier, identifier.c_str() );	*/
	*deviceList = NULL;
	numDevices = 0;
}

//extern "C" VMHIGHGUI_API void freeAvailableDevices( VMInputIdentification **deviceList, int &numDevices );
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

//extern "C" VMHIGHGUI_API const char **getIdentifiers( int &numIdentifiers );
const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

//extern "C" VMHIGHGUI_API char* checkVersion( );
char* checkVersion( )
{
	#ifdef _DEBUG
		return "DEBUG" ;			
	#endif
	#ifdef NDEBUG
		return "RELEASE";			
	#endif
}

//extern "C" VMHIGHGUI_API void deleteInput( VideoInput **input );
void deleteInput( VideoInput **input )
{
	delete (*input);
	(*input) = NULL;
}
