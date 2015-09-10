#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include "ImageSequence.h"
#include <vector>

using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"IMAGE_SEQUENCE"};
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

//extern "C" IMG_SEQ_API VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format );
VideoInput *initVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	ImgSeq *imgSeq = new ImgSeq();
	if ( imgSeq->initSequence(device.fileName, format ) )		
		return imgSeq;	
	delete imgSeq;
	return NULL;
}

//extern "C" IMG_SEQ_API void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices );
void getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
	numDevices = 0;
}

void freeAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
{
}

//extern "C" IMG_SEQ_API const char **getIdentifiers( int &numIdentifiers );
const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

//extern "C" IMG_SEQ_API char* checkVersion( );
char* checkVersion( )
{
	#ifndef NDEBUG
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
