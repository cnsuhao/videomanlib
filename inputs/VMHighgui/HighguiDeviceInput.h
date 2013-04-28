#pragma once
#include "VideoManInputFormat.h"
#include "VideoInput.h"

#include "opencv2/highgui/highgui.hpp"

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
	cv::VideoCapture capture;
};
