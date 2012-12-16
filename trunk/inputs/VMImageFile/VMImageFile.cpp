#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include "ImageFile.h"
#include <vector>

using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"IMAGE_FILE"};
const int numIdentifiers = 1;

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
	ImageFile *imgFile = new ImageFile();
	if ( imgFile->initInput(device.fileName, format ) )		
		return imgFile;	
	delete imgFile;
	return NULL;
}

void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	numDevices = 0;
}

void freeAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
}

const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

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
