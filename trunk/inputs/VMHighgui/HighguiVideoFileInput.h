#pragma once
#include "VideoManInputFormat.h"
#include "VideoInput.h"
#include <highgui.h>


class HighguiVideoFileInput :
	public VideoManPrivate::VideoInput
{
public:
	HighguiVideoFileInput(void);
	virtual ~HighguiVideoFileInput(void);

	bool initInput( const VideoMan::VMInputIdentification &device, VideoMan::VMInputFormat *format );
		
	char *getFrame( bool wait = false);

	void releaseFrame( );	
		
	double getLengthSeconds();
	
	int getLengthFrames();

	double getPositionSeconds();
	
	int getPositionFrames();

	void goToFrame( int frame );
	
	void goToMilisecond( double milisecond );

private:	
	CvCapture *capture;
};
