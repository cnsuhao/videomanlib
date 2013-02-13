#define _CRT_SECURE_NO_DEPRECATE //warning C4996: 'strcpy' was declared deprecated

#include <iostream>
#include <string>
#include "VideoManFactory.h"

#ifdef linux
	#include <stddef.h>
	#include <sys/stat.h>
	#include <stdio.h>
	#include <link.h>
	#include <sys/types.h>
	#include <dirent.h>
    #include <string.h>
#endif

using namespace std;
using namespace VideoMan;
using namespace VideoManPrivate;

void copyStringToChar( const string &src, char **dst )
{
	if ( src.length() > 0 )
	{
		*dst = new char[src.length() + 1];
		strcpy( *dst, src.c_str() );
	}
	else
		*dst = NULL;
}

void cloneCharToChar( const char *src, char **dst )
{
	if ( src != NULL )
		copyStringToChar( src, dst );
	else
		*dst = NULL;
}

void freeChar( char **src )
{
	if ( *src )
	{
		delete *src;
		*src = NULL;
	}
}

void cloneInputIdentification( const VMInputIdentification &src, VMInputIdentification &dst )
{
	cloneCharToChar( src.uniqueName, &dst.uniqueName );
	cloneCharToChar( src.fileName, &dst.fileName );
	cloneCharToChar( src.friendlyName, &dst.friendlyName );
	cloneCharToChar( src.identifier, &dst.identifier );	
}

void cloneDeviceList( const VMInputIdentification *src, VMInputIdentification *&dst, const int numDevices )
{
	dst = NULL;
	if ( numDevices != 0 )
		dst = new VMInputIdentification[numDevices];
	for ( int d = 0; d < numDevices; ++d )
	{
		cloneInputIdentification( src[d], dst[d] );
	}
}

VideoManFactory::VideoManFactory(void)
{
	modulesLoaded = false;
}

void VideoManFactory::deleteInput( VideoInput **input )
{	
	const string identifier = (*input)->getIdentification().identifier;
	if ( identifier == "USER_INPUT" )
	{
		delete (*input);
		*input = NULL;
		return;
	}
	else if ( !modulesLoaded )
		loadModules();
	if ( identifiersMap.find( identifier ) == identifiersMap.end() )
		return;
	std::string moduleName = identifiersMap[identifier];
	#ifdef WIN32		
		HINSTANCE hinstLib = getLoadedModuleHandle( moduleName );
		if ( hinstLib != NULL )
		{
			typedef void (*importFunction)( VideoInput** );
			importFunction deleteInput = (importFunction)GetProcAddress(hinstLib, "deleteInput");
			if ( deleteInput != NULL )
				deleteInput( input );
		}
	#endif
	#ifdef linux		
		void *handle = getLoadedModuleHandle( moduleName );
		if ( handle )
		{
			typedef void (*importFunction)( VideoInput** );
			importFunction deleteInput2 = (importFunction)dlsym(handle, "deleteInput");
			char *error=NULL;
			error = dlerror();
			if (error == NULL)
			{
				(*deleteInput2)( input );
			}
			else
			{
				printf("Error: [%s]\n",error);
			}
		}
	#endif
}

void VideoManFactory::freeModules()
{
	#ifdef WIN32
		/*for ( size_t m = 0; m< moduleList.size(); ++m )
		{
			HINSTANCE hinstLib = getLoadedModuleHandle( moduleList[m] );
			if ( hinstLib != NULL )
			{
				typedef void (*importFunction)();
				importFunction deleteInputs = (importFunction)GetProcAddress(hinstLib, "deleteInputs");
				if ( deleteInputs != NULL )
					deleteInputs();
				importFunction deleteControllers = (importFunction)GetProcAddress(hinstLib, "deleteControllers");
				if ( deleteControllers != NULL )
					deleteControllers();
			}
		}*/
		for ( loadedModulesIter m = loadedModules.begin(); m != loadedModules.end(); ++m )
			FreeLibrary( (*m).second);
	#endif
	#ifdef linux
		/*for ( size_t m = 0; m< moduleList.size(); ++m )
		{
			void *handle = getLoadedModuleHandle( moduleList[m] );
			if ( handle )
			{
				typedef void (*importFunction)();
				importFunction deleteInputs = (importFunction)dlsym(handle, "deleteInputs");
				char *error;
				if ((error = dlerror()) != NULL)
					(*deleteInputs)();

				importFunction deleteControllers = (importFunction)dlsym(handle, "deleteControllers");
				if ((error = dlerror()) != NULL)
					(*deleteControllers)();
			}
		}*/
		for ( loadedModulesIter m = loadedModules.begin(); m != loadedModules.end(); ++m )
			dlclose((*m).second);
	#endif
	loadedModules.clear();
	modulesLoaded = false;
}

VideoManFactory::~VideoManFactory(void)
{
	freeModules();
}

void VideoManFactory::freeAvailableDevicesList( VMInputIdentification **deviceList, int &numDevices )
{
	if ( *deviceList == NULL || numDevices == 0 )
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

void VideoManFactory::LoadModulesWindows( const std::string &path )
{
#ifdef WIN32
	//char path2[MAX_PATH];
	WIN32_FIND_DATA fd;
	DWORD dwAttr = FILE_ATTRIBUTE_ARCHIVE;
	//sprintf( path2, "%s\\*", path.c_str() );

	const std::string modulesPath = path + "\\vm*.dll";
	HANDLE hFind = FindFirstFile( modulesPath.c_str(), &fd);
	if(hFind == INVALID_HANDLE_VALUE)
	{
		FindClose( hFind);
		return;
	}
	do
	{
		//if ( fd.dwFileAttributes & dwAttr )
		{
			std::string fileName = path + "\\" + fd.cFileName;
			if ( fileName.rfind(".dll") != std::string::npos )
			{
				cout << "loading " << fileName << endl;
				HINSTANCE hinstLib = LoadLibrary(  fileName.c_str() );
				if ( !hinstLib )
				{
					cout << "Error loading " << fileName << endl;
					continue;
				}

				//Check the version
				typedef char* (*checkVersionImportFunction)();
				checkVersionImportFunction checkVersion = (checkVersionImportFunction)GetProcAddress(hinstLib, "checkVersion");
				if ( checkVersion == NULL )
					continue;
				string version = checkVersion();				
				#ifdef _DEBUG
					if  ( version != "DEBUG" )
					{
						FreeLibrary(hinstLib);
						continue;
					}
				#endif
				#ifdef NDEBUG
					if  ( version != "RELEASE" )
					{
						FreeLibrary(hinstLib);
						continue;
					}
				#endif

				//Get the module indentifiers
				typedef const char **(*getIdentifiersImportFunction)( int &numIdentifiers );
				getIdentifiersImportFunction getIdentifiers = (getIdentifiersImportFunction)GetProcAddress(hinstLib, "getIdentifiers");
				if ( getIdentifiers != NULL )
				{
					//std::vector< string > identifierList;
					int numIdentifiers;
					const char **identifierList = getIdentifiers( numIdentifiers );
					for ( int i = 0; i < numIdentifiers; ++i )
					{
						identifiersMap[identifierList[i]] = fileName;
					}
					moduleList.push_back( fileName );
				}

				//Get the controller identifiers
				getIdentifiersImportFunction getControllerIdentifiers = (getIdentifiersImportFunction)GetProcAddress(hinstLib, "getControllerIdentifiers");
				if ( getControllerIdentifiers != NULL )
				{
					//std::vector< string > controllersList;
					int numControllers;
					const char **controllersList = getControllerIdentifiers( numControllers );
					for ( int i = 0; i < numControllers; ++i )
					{
						controllersMap[controllersList[i]] = fileName;
					}
				}

				FreeLibrary(hinstLib);
			}
		}
	}while( FindNextFile( hFind, &fd));
	FindClose( hFind);
#endif
}

void VideoManFactory::LoadModulesLinux( const std::string &path )
{
#ifdef linux
	//const std::string modulesPath = path + "\\vm*.so";
	void *handle;
    char *error;

	DIR *dir;
	struct dirent *dent;
	struct stat stbuf;
	char buf[PATH_MAX];

	if(!(dir = opendir(path.c_str())))
	{
		perror("opendir()");
		return;
	}
	while( dent = readdir( dir ) )
	{
		std::string fileName = dent->d_name;
		if ( fileName.rfind(".so") != std::string::npos )
		{
			handle = dlopen( dent->d_name, RTLD_LAZY );
			if ( !handle )
			{
				cout << "Error loading " << dent->d_name << endl;
				continue;
			}
			//Check the version
			typedef char* (*checkVersionImportFunction)();
			checkVersionImportFunction checkVersion = (checkVersionImportFunction)dlsym( handle, "checkVersion" );
			if ((error = dlerror()) != NULL)
				continue;
			string version = (*checkVersion)();
			#ifdef _DEBUG
				if  ( version != "DEBUG" )
				{
					dlclose(handle);
					continue;
				}
			#endif
			#ifdef NDEBUG
				if  ( version != "RELEASE" )
				{
					dlclose(handle);
					continue;
				}
			#endif

			//Get the module indentifiers
			typedef char ** (*getIdentifiersImportFunction)( int & );
			getIdentifiersImportFunction getIdentifiers = (getIdentifiersImportFunction)dlsym( handle, "getIdentifiers" );
			if ((error = dlerror()) == NULL)
			{
				int numIdentifiers;
				char **identifierList = (*getIdentifiers)( numIdentifiers );
				for ( size_t i = 0; i < numIdentifiers; ++i )
				{
					identifiersMap[identifierList[i]] = fileName;
				}
				moduleList.push_back( fileName );
			}

			//Get the controller identifiers
			getIdentifiersImportFunction getControllerIdentifiers = (getIdentifiersImportFunction)dlsym( handle, "getControllerIdentifiers" );
			if ( getControllerIdentifiers != NULL )
			{
				int numIdentifiers;
				char **controllersList = (*getControllerIdentifiers)( numIdentifiers );
				for ( size_t i = 0; i < numIdentifiers; ++i )
				{
					controllersMap[controllersList[i]] = fileName;
				}
			}

			dlclose(handle);
		}
	}
	closedir( dir );
#endif
}


void VideoManFactory::loadModules()
{
	std::string path;
	#ifdef WIN32
		#ifdef _DEBUG
			HINSTANCE hInstance = GetModuleHandle("videomand.dll");
		#endif
		#ifdef NDEBUG
			HINSTANCE hInstance = GetModuleHandle("videoman.dll");
		#endif
		const DWORD bufSize = 512;
		char szFileName[bufSize];
		DWORD pathLength = GetModuleFileName(hInstance,szFileName,bufSize);
		path = szFileName;
		//remove videoman.dll from the path
		std::string::size_type index;
		index = path.find_last_of("\\");
		if ( index != std::string::npos )
			path = path.substr( 0, index );
	#endif
	#ifdef linux
		void *handle = dlopen("libVideoMan.so", RTLD_LAZY); 
		link_map *map;
		if ( dlinfo( handle,  RTLD_DI_LINKMAP, &map ) != -1 )
		{
			path = map->l_name;
			std::string::size_type index;
			index = path.find_last_of("/");
			if ( index != std::string::npos )
				path = path.substr( 0, index );
		}
		else
		{
			pid_t pid = getpid();
			char buf[10];
			sprintf(buf,"%d",pid);
			std::string _link = "/proc/";
			_link.append( buf );
			_link.append( "/exe");
			char proc[512];
			int ch = readlink(_link.c_str(),proc,512);
			if (ch != -1) 
			{
				proc[ch] = 0;
				path = proc;
				std::string::size_type t = path.find_last_of("/");
				path = path.substr(0,t);
			}
			std::cout << "You must copy the vm*.so to your binary path" << std::endl;
		}
	#endif
	std::cout << "Videoman loaded from " << path << std::endl;

	moduleList.clear();
	identifiersMap.clear();
	controllersMap.clear();

	#ifdef WIN32
		LoadModulesWindows( path );
	#endif
	#ifdef linux
		LoadModulesLinux( path );
	#endif

	//Print info
	cout << "Number of loaded input modules " <<  moduleList.size() << endl;
	for ( size_t m = 0; m < moduleList.size(); ++m )
		cout << moduleList[m] << endl;

	cout << "Number of available identifiers " <<  identifiersMap.size() + 1 << endl;
	cout << "USER_INPUT" << endl;
	std::map< std::string, std::string >::iterator it;
	for ( it = identifiersMap.begin(); it != identifiersMap.end(); ++it )
		cout << it->first << endl;

	cout << "Number of available controllers " <<  controllersMap.size() << endl;
	for ( it = controllersMap.begin(); it != controllersMap.end(); ++it )
	{
		cout << it->first << endl;
	}
	modulesLoaded = true;
}

VideoInput *VideoManFactory::createUserInput( VMInputFormat *format )
{
	if ( format == NULL )
	{
		cerr << "createUserInput: Must specify the format"  << endl;
		return NULL;
	}
	UserInput *userInput = new UserInput();
	if ( !userInput->init( *format ) )
	{
		delete userInput;
		return NULL;
	}

	return userInput;
}

VideoInput *VideoManFactory::createVideoInputWindows( const VMInputIdentification &device, VMInputFormat *format )
{
#ifdef WIN32
	VideoInput *input;
	//Check if the identifier is known
	if ( identifiersMap.find( device.identifier ) == identifiersMap.end() )
	{
		cerr <<"ERROR: unknown identifier " << device.identifier << endl;
		return NULL;
	}
	std::string moduleName = identifiersMap[device.identifier];

	//Check if the module was already loaded
	HINSTANCE hinstLib = getLoadedModuleHandle( moduleName );
	if ( hinstLib == NULL )
		hinstLib = LoadLibrary( moduleName.c_str() );
	if ( hinstLib == NULL )
	{
		cerr << "ERROR: unable to load DLL "  << moduleName << endl;
		return NULL;
	}

	typedef VideoInput* (*importFunction)( const VMInputIdentification &device, VMInputFormat *format );
	importFunction initFunc = (importFunction)GetProcAddress(hinstLib, "initVideoInput");
	if ( initFunc == NULL )
	{
		FreeLibrary(hinstLib);
		cerr << "ERROR: unable to find initVideoInput DLL function in "  << moduleName << endl ;
		return NULL;
	}

	input = initFunc( device, format );
	if ( input == NULL )
		return NULL;
	loadedModules[moduleName] = hinstLib;
	return input;
#endif
	return NULL;
}

VideoInput *VideoManFactory::createVideoInputLinux( const VMInputIdentification &device, VMInputFormat *format )
{
#ifdef linux
	VideoInput *input;
	//Check if the identifier is known
	if ( identifiersMap.find( device.identifier ) == identifiersMap.end() )
	{
		cerr <<"ERROR: unknown identifier " << device.identifier << endl;
		return NULL;
	}

	std::string moduleName = identifiersMap[device.identifier];

	//Check if the module was already loaded
	void *handle = getLoadedModuleHandle( moduleName );
	if ( !handle )
		handle = dlopen( moduleName.c_str(), RTLD_LAZY);
	if ( !handle )
	{
		cerr << "ERROR: unable to load library "  << moduleName << endl ;
		return NULL;
	}

	typedef VideoInput* (*importFunction)( const VMInputIdentification &device, VMInputFormat *format );
	importFunction initFunc = (importFunction)dlsym(handle, "initVideoInput");
	char *error;
	if ((error = dlerror()) != NULL)
	{
		fputs(error, stderr);
		cerr << "ERROR: unable to find initVideoInput library function in "  << moduleName << endl ;
		dlclose(handle);
		return NULL;
	}
	input = (*initFunc)( device, format );
	if ( input == NULL )
		return NULL;
	loadedModules[moduleName] = handle;
	return input;
#endif
	return NULL;
}

VideoInput *VideoManFactory::createVideoInput( const VMInputIdentification &device, VMInputFormat *format )
{
	if ( device.identifier == NULL )
		return NULL;
	string identifier = device.identifier;
	if ( identifier == "USER_INPUT" )
	{
		VideoInput *input;
		input = createUserInput( format );
		return input;
	}
	else if ( !modulesLoaded )
		loadModules();
	#ifdef WIN32
		return createVideoInputWindows( device, format );
	#endif
	#ifdef linux
		return createVideoInputLinux( device, format );
	#endif
}

#ifdef WIN32
HINSTANCE VideoManFactory::getLoadedModuleHandle( std::string moduleName )
{
    if ( loadedModules.find( moduleName ) != loadedModules.end() )
	{
		return loadedModules[moduleName];
	}
	return NULL;
}
#endif

#ifdef linux
void *VideoManFactory::getLoadedModuleHandle( std::string moduleName )
{
    if ( loadedModules.find( moduleName ) != loadedModules.end() )
	{
		return loadedModules[moduleName];
	}
	return NULL;
}
#endif



void VideoManFactory::getAvailableDevices( VMInputIdentification **deviceList, int &numDevices )
//void VideoManFactory::getAvailableDevices( std::vector<VMInputIdentification> &deviceList )
{
	if ( !modulesLoaded )
		loadModules();
	*deviceList = NULL;
	numDevices = 0;

	int totalDevices = 0;
	vector< pair< int, VMInputIdentification*> > deviceLists;
	for ( size_t m = 0; m< moduleList.size(); ++m )
	{
		VMInputIdentification *list;
		int numDevicesModule = 0;
		getAvailableDevicesFromModule( moduleList[m], &list, numDevicesModule );
		if ( numDevicesModule > 0 )
		{
			deviceLists.push_back( pair< int, VMInputIdentification*>(numDevicesModule, list) );
			totalDevices += numDevicesModule;
		}
	}
	if( totalDevices > 0 )
	{
		numDevices = totalDevices;
		(*deviceList) = new VMInputIdentification[numDevices];
		//Insert the devices into the complete list
		int deviceInd = 0;
		for ( size_t t = 0; t < deviceLists.size(); ++t )
		{
			VMInputIdentification *list = deviceLists[t].second;
			for ( int d = 0; d < deviceLists[t].first; ++d )
			{
				//cloneInputIdentification( list[d], availableDevices[deviceInd] );
				(*deviceList)[deviceInd] = list[d];
				++deviceInd;
			}
			//freeAvailableDevicesList( &list, deviceLists[t].first );
			delete list;
		}
	}
}

void VideoManFactory::getAvailableDevices( const char *identifier, VMInputIdentification **deviceList, int &numDevices )
//void VideoManFactory::getAvailableDevices( const std::string &identifier, std::vector<VMInputIdentification> &deviceList )
{
	if ( !modulesLoaded )
		loadModules();
	numDevices = 0;
	*deviceList = NULL;

    //Check if the identifier is known
	if ( identifiersMap.find( identifier )== identifiersMap.end() )
		return;
    getAvailableDevicesFromModule( identifiersMap[identifier], &(*deviceList), numDevices );
}

#ifdef WIN32
void VideoManFactory::getAvailableDevicesFromModule( const std::string &moduleName, VMInputIdentification **deviceList, int &numDevices )
{
	//Check if the module was already loaded
	bool freeModule = false;
	HINSTANCE hinstLib = getLoadedModuleHandle( moduleName );
	if ( hinstLib == NULL )
	{
		hinstLib = LoadLibrary( moduleName.c_str() );
		freeModule = true;
	}
	if ( hinstLib == NULL )
	{
		cerr << "ERROR: unable to load DLL"  << endl;
		return;
	}

	typedef void (*importFunction)( VMInputIdentification **deviceList, int &numDevices );//std::vector<VMInputIdentification> &deviceList );
	importFunction getAvailableDevices = (importFunction)GetProcAddress(hinstLib, "getAvailableDevices");
	if ( getAvailableDevices == NULL )
	{
		cerr << "ERROR: unable to find getAvailableDevices DLL function in " << moduleName << endl;
		if ( freeModule )
			FreeLibrary(hinstLib);
		return;
	}
	typedef void (*importFunction)( VMInputIdentification **deviceList, int &numDevices );//std::vector<VMInputIdentification> &deviceList );
	importFunction freeAvailableDevices = (importFunction)GetProcAddress(hinstLib, "freeAvailableDevices");
	if ( freeAvailableDevices == NULL )
	{
		cerr << "ERROR: unable to find freeAvailableDevices DLL function in " << moduleName << endl;
		if ( freeModule )
			FreeLibrary(hinstLib);
		return;
	}

	//Get the module devices
	VMInputIdentification *newDevices = NULL;
	numDevices = 0;
	int numDevicesModule;
	getAvailableDevices( &newDevices, numDevicesModule );
	cloneDeviceList( newDevices, *deviceList, numDevicesModule );
	numDevices = numDevicesModule;
	freeAvailableDevices( &newDevices, numDevicesModule );

	if ( freeModule )
		FreeLibrary(hinstLib);
}
#endif

#ifdef linux
void VideoManFactory::getAvailableDevicesFromModule( const std::string &moduleName, VMInputIdentification **deviceList, int &numDevices )
{
	//Check if the module was already loaded
	bool freeModule = false;
	void *handle = getLoadedModuleHandle( moduleName );
	if ( !handle )
	{
		handle = dlopen( moduleName.c_str(), RTLD_LAZY);
		freeModule = true;
	}
	if ( !handle )
	{
		cerr << "ERROR: unable to load library "  << moduleName << endl ;
		return;
	}

	typedef void (*importFunction)(  VMInputIdentification **deviceList, int &numDevices );
	importFunction getAvailableDevices = (importFunction)dlsym(handle, "getAvailableDevices");
	char *error;
	if ((error = dlerror()) != NULL)
	{
		fputs(error, stderr);
		cerr << "ERROR: unable to find getAvailableDevices library function in "  << moduleName << endl ;
		if ( freeModule )
			dlclose(handle);
		return;
	}

	typedef void (*importFunction)( VMInputIdentification **deviceList, int &numDevices );
	importFunction freeAvailableDevices = (importFunction)dlsym(handle, "freeAvailableDevices");
	//char *error;
	if ((error = dlerror()) != NULL)
	{
		fputs(error, stderr);
		cerr << "ERROR: unable to find freeAvailableDevices library function in "  << moduleName << endl ;
		if ( freeModule )
			dlclose(handle);
		return;
	}


	//Get the module devices
	VMInputIdentification *newDevices = NULL;
	numDevices = 0;
	int numDevicesModule;
	(*getAvailableDevices)( &newDevices, numDevicesModule );
	cloneDeviceList( newDevices, *deviceList, numDevicesModule );
	numDevices = numDevicesModule;
	(*freeAvailableDevices)( &newDevices, numDevicesModule );

	if ( freeModule )
		dlclose(handle);
}
#endif

VideoManInputController *VideoManFactory::createController( const char *controllerIdentifier )
{
	if ( !modulesLoaded )
		loadModules();
	if ( controllerIdentifier == NULL)
		return NULL;
	#ifdef WIN32
		return createControllerWindows( controllerIdentifier );
	#endif
	#ifdef linux
		return createControllerLinux( controllerIdentifier );
	#endif
	return NULL;
}

void VideoManFactory::deleteController( VideoManInputController **controller )
{
	if ( !modulesLoaded )
		loadModules();

    typedef void (*importFunction)( VideoManInputController** );

	#ifdef WIN32
		const string identifier = (*controller)->getIdentifier();
		if ( controllersMap.find( identifier ) == controllersMap.end() )
			return;
		std::string moduleName = controllersMap[identifier];
		HINSTANCE hinstLib = getLoadedModuleHandle( moduleName );
		if ( hinstLib != NULL )
		{
			importFunction deleteController = (importFunction)GetProcAddress(hinstLib, "deleteController");
			if ( deleteController != NULL )
				deleteController( controller );
		}
	#endif
	#ifdef linux
		const string identifier = (*controller)->getIdentifier();
		if ( controllersMap.find( identifier ) == controllersMap.end() )
			return;
		std::string moduleName = controllersMap[identifier];
		void *handle = getLoadedModuleHandle( moduleName );
		if ( handle )
		{
			importFunction deleteController = (importFunction)dlsym(handle, "deleteController");
			char *error;
			if ((error = dlerror()) != NULL)
				(*deleteController)( controller );
		}
	#endif
}


VideoManInputController *VideoManFactory::createControllerLinux( const std::string &controllerIdentifier )
{
	#ifdef linux
	//Check if the controller identifier is known
	if ( controllersMap.find( controllerIdentifier ) != controllersMap.end() )
	{
		//Get the module name
		string moduleName = controllersMap[controllerIdentifier];
		//Check if the module was already loaded
		void *handle = getLoadedModuleHandle( moduleName );
		if ( !handle )
			handle = dlopen( moduleName.c_str(), RTLD_LAZY);
		if ( !handle )
		{
			cerr << "ERROR createControllerLinux: unable to load library "  << moduleName << endl ;
			return NULL;
		}

		//Get createController function
		typedef VideoManInputController* (*importFunction)( const char *identifier );
		importFunction createController = (importFunction)dlsym(handle, "createController");
		char *error;
		if ((error = dlerror()) != NULL)
		{
			fputs(error, stderr);
			cerr << "ERROR: unable to find createController library function in "  << moduleName << endl ;
			dlclose(handle);
			return NULL;
		}
		VideoManInputController *controller = (*createController)( controllerIdentifier.c_str() );
		return controller;
	}
	#endif
	return NULL;
}

VideoManInputController *VideoManFactory::createControllerWindows( const std::string &controllerIdentifier )
{
	#ifdef WIN32
	//Check if the controller identifier is known
	if ( controllersMap.find( controllerIdentifier ) != controllersMap.end() )
	{
		//Get the module name
		string moduleName = controllersMap[controllerIdentifier];
		//Check if the module was already loaded
		HINSTANCE hinstLib = getLoadedModuleHandle( moduleName );
		if ( hinstLib == NULL )
			hinstLib = LoadLibrary( moduleName.c_str() );
		if ( hinstLib == NULL )
		{
			cerr << "ERROR createController: unable to load DLL" <<  moduleName << endl;
			return NULL;
		}

		//Get createController function from dll
		typedef VideoManInputController*(*importFunction)( const char *identifier );
		importFunction createController = (importFunction)GetProcAddress(hinstLib, "createController");
		if ( createController == NULL )
		{
			cerr << "ERROR createController: unable to find createController DLL function in " << moduleName << endl;
			return NULL;
		}

		VideoManInputController *controller = createController( controllerIdentifier.c_str() );
		return controller;
	}
	#endif
	return NULL;
}

void VideoManFactory::getSupportedIdentifiers( char **&identifiers, int &numIdentifiers )
{
	if ( !modulesLoaded )
		loadModules();
	identifiers = new char*[identifiersMap.size()];
	std::map< std::string, std::string >::iterator it;
	numIdentifiers = 0;
	for ( it = identifiersMap.begin(); it != identifiersMap.end(); ++it )
	{
		copyStringToChar( it->first, &identifiers[numIdentifiers] );		
		++numIdentifiers;
	}
}

void VideoManFactory::freeSupportedIdentifiersList( char **&identifiers, int &numIdentifiers )
{
	if ( *identifiers == NULL || numIdentifiers == 0 )
		return;
	for ( int d = 0; d < numIdentifiers; ++d )
		freeChar( &identifiers[d] );
	delete *identifiers;
	*identifiers = NULL;
	numIdentifiers = 0;
}

bool VideoManFactory::supportedIdentifier( const char *_identifier )
{
	if ( !modulesLoaded )
		loadModules();
	const string identifier = _identifier;
	std::map< std::string, std::string >::iterator it;
	for ( it = identifiersMap.begin(); it != identifiersMap.end(); ++it )
	{
		if ( it->first == identifier )
			return true;
	}
	return false;
}
