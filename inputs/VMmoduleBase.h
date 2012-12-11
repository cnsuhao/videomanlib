#ifndef VMMODULEBASE_H
#define VMMODULEBASE_H

#include "VideoManInputFormat.h"
#include "VideoInput.h"

#ifdef WIN32
//#ifdef VideoMan_EXPORTS
#define VMMODULE_API __declspec(dllexport)
//#else
//#define VMMODULE_API __declspec(dllimport)
//#endif
#endif
#ifdef linux
#define VMMODULE_API
#endif

extern "C"
{
/**
INPUTS CREATION METHOD
*/
VMMODULE_API VideoManPrivate::VideoInput *initVideoInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );


/**
METHOD FOR KNOWING THE LIST OF DEVICES THE MODULE CAN HANDLE
THIS METHOD COULD NOT BE DECLARED FOR VIDEO FILE INPUT MODULES FOR EXAMPLE
*/
VMMODULE_API void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices );

/**
METHOD FOR RELEASING THE MEMORY USED FOR THE DEVICE LIST
THIS METHOD COULD NOT BE DECLARED FOR VIDEO FILE INPUT MODULES FOR EXAMPLE
*/
VMMODULE_API void freeAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices );

/**
METHOD FOR KNOWING THE LIST OF IDENTIFIERS THE MODULE CAN HANDLE
*/
VMMODULE_API const char **getIdentifiers( int &numIdentifiers );

/**
METHOD FOR KNOWING THE LIST OF CONTROLLERS IDENTIFIERS
*/
VMMODULE_API const char **getControllerIdentifiers( int &numIdentifiers );

/**
FIXED METHOD FOR CHECKING THE VERSION
*/
VMMODULE_API char* checkVersion( );

/**
METHOD FOR CREATING A CONTROLLER
*/
VMMODULE_API VideoManPrivate::VideoManInputController *createController( const char *identifier  );

/**
METHOD FOR DELETING A INPUT CREATED BY THIS MODULE
*/
VMMODULE_API void deleteInput( VideoManPrivate::VideoInput **input );

/**
METHOD FOR DELETING A CONTROLLER CREATED BY THIS MODULE
*/
VMMODULE_API void deleteController( VideoManPrivate::VideoManInputController **controller );
}

#endif /*VMMODULEBASE_H*/
