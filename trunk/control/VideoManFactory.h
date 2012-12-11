#ifndef VIDEOMANFACTORY_H
#define VIDEOMANFACTORY_H

#ifdef WIN32
	#include <windows.h>
#endif
#ifdef linux
	#include <dlfcn.h> 
#endif
#include <map>
#include <vector>
//#include "VideoManControl.h"
#include "VideoInput.h"
#include "inputs/UserInput.h"

namespace VideoManPrivate
{
class VideoManFactory
{
	VideoInput *createUserInput( VideoMan::VMInputFormat *format );

	void getAvailableDevicesFromModule( const std::string &moduleName, VideoMan::VMInputIdentification **deviceList, int &numDevices );
	#ifdef WIN32
		HINSTANCE getLoadedModuleHandle( std::string moduleName );
	#endif
	#ifdef linux
		void *getLoadedModuleHandle( std::string moduleName );
	#endif

	void LoadModulesWindows( const std::string &path );
	void LoadModulesLinux( const std::string &path );

	VideoInput *createVideoInputWindows( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );
	VideoInput *createVideoInputLinux( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );

	VideoManPrivate::VideoManInputController *createControllerLinux( const std::string &controllerIdentifier );
	VideoManPrivate::VideoManInputController *createControllerWindows( const std::string &controllerIdentifier );

	std::map< std::string, std::string > identifiersMap;
	std::map< std::string, std::string > controllersMap;
	#ifdef WIN32
		std::map< std::string, HINSTANCE > loadedModules;
		typedef std::map< std::string, HINSTANCE >::iterator loadedModulesIter;
	#endif
	#ifdef linux
		std::map< std::string, void* > loadedModules;
		typedef std::map< std::string, void* >::iterator loadedModulesIter;
	#endif

	void loadModules( );

	std::vector< std::string > moduleList;
	bool modulesLoaded;	

public:
	VideoManFactory(void);
	~VideoManFactory(void);	

	void freeModules();

	void deleteInput( VideoInput **input );

	void getSupportedIdentifiers( char **&identifiers, int &numIdentifiers );

	void freeSupportedIdentifiersList( char **&identifiers, int &numIdentifiers );

	bool supportedIdentifier( const char *identifier );

	VideoInput *createVideoInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );
	
	void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices );
	//void getAvailableDevices( std::vector<VMInputIdentification> &deviceList );

	void getAvailableDevices( const char *identifier, VideoMan::VMInputIdentification **deviceList, int &numDevices );
	//void getAvailableDevices( const std::string &identifier, std::vector<VMInputIdentification> &deviceList );

	void freeAvailableDevicesList( VideoMan::VMInputIdentification **deviceList, int &numDevices );

	VideoManPrivate::VideoManInputController *createController( const char *controllerIdentifier );

	void deleteController( VideoManPrivate::VideoManInputController **controller );
};
};
#endif