#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include <vector>
#include "TISInput.h"


using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"TIS_CAMERA"};
const int numIdentifiers = 1;
const char *controllersIdentifiers[1] = {"TIS_CAMERA_CONTROLLER"};
const int numControllersIdentifiers = 1;

#ifdef WIN32
#define VMImagingSource_API  __declspec(dllexport)
#elif defined linux
#define VMImagingSource_API  
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
		{
			IC_InitLibrary(0);
			break;
		}
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		{
			IC_CloseLibrary();
			break;
		}
	}

    return TRUE;
}
#endif

//extern "C" VMIDSuEye_API VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format );
VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	TISinput *input = new TISinput();
	if ( input->initInput( device, format ) )
		return input;
	else
		delete input;
	return NULL;

}

//extern "C" VMIDSuEye_API void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices );
void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	TISinput::getAvailableDevices( deviceList, numDevices );
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

	return NULL;
}

//extern "C" VMIDSuEye_API void deleteController( VideoManInputController **controller );
void deleteController( VideoManInputController **controller )
{
	delete (*controller);
	(*controller) = NULL;
}
