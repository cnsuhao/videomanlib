#pragma once
#include "VideoManInputFormat.h"
#include "VideoInput.h"

#include <highgui.h>


class HighguiDeviceInput :
	public VideoManPrivate::VideoInput
{
public:
	HighguiDeviceInput(void);
	virtual ~HighguiDeviceInput(void);

	bool initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );
		
	char *getFrame( bool wait = false);

	void releaseFrame( );	

private:	
	CvCapture *capture;
};
