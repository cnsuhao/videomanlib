#ifndef YourInputClass_H
#define YourInputClass_H

#include "VideoInput.h"
#include "VideoManInputFormat.h"
class VMYOURINPUTNAMEInput :
	public VideoManPrivate::VideoInput
{
public:
	VMYOURINPUTNAMEInput(void);
	virtual ~VMYOURINPUTNAMEInput(void);

	//-- mandatory --//
	char *getFrame( bool wait = false);

	//-- Optionals --//
	void releaseFrame();
	
	void play();
	
	void pause();
	
	void stop();

	static void getAvailableDevices( VideoMan::VMInputIdentification **deviceList, int &numDevices  );	

	bool init( const char *friendlyName, const char *devicePath, VideoMan::VMInputFormat *aFormat );


private:

};

#endif