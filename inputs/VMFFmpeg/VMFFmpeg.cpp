#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include "VMmoduleBase.h"
#include "videofile.h"
#include <vector>

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

const char *identifiers[1] = {"FFMPEG_VIDEO_FILE"};
const int numIdentifiers = 1;
//const char *controllersIdentifiers[] = {""};
const int numControllersIdentifiers = 0;

#ifdef WIN32
#define VMFFmpeg_API  __declspec(dllexport)
#elif defined linux
#define VMFFmpeg_API
#endif

void freeChar( char **src )
{
	if ( *src )
	{
		delete [] *src;
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
	VideoFile *videoFile = new VideoFile();
	if ( videoFile->initInput( device, format ) )
	{
		return videoFile;
	}
	else
	{
		delete videoFile;
		return NULL;
	}
}

const char **getIdentifiers( int &_numIdentifiers )
{
	_numIdentifiers  = numIdentifiers;
	return identifiers;
}

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